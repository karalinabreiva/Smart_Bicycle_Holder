//--------------------------------------------------------------------------------------------
//                                         LIBRARIES
//--------------------------------------------------------------------------------------------
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>


//--------------------------------------------------------------------------------------------
//                                         VARIABLES
//--------------------------------------------------------------------------------------------
#define SS_PIN D4
#define RST_PIN D3
#define SERVO_PIN D1

MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

Servo myservo;
int state = 1;

//const String password = "1234"; // change password
String input_password;

typedef struct struct_message {
  int ID;
  bool isOpen;
  int codeFromUser;
  } struct_message;
const char* ssid = "PLAY_Swiatlowodowy_12E2";
const char* password = "4fE7vqetqVe#";
String serverName = "http://192.168.0.37:3001";

struct_message myData;


//--------------------------------------------------------------------------------------------
//                                         SETUP (RUN ONCE)
//--------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  
  
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  Serial.println();
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.println();
  Serial.println(F("This code scan the MIFARE Classic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  myservo.attach(SERVO_PIN);
  myservo.write(0);
}


//--------------------------------------------------------------------------------------------
//                                         MAIN LOOP (RUN REPEATEDLY)
//--------------------------------------------------------------------------------------------
void loop() {
  getDevicesData(&myData,0);
  
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("Your tag is not of type MIFARE Classic."));
      return;
  }

    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++)
    {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    check(&myData);
    closeRemotely(&myData);

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


//--------------------------------------------------------------------------------------------
//                                         FUNCTIONS
//--------------------------------------------------------------------------------------------
void sendUnlock(struct_message* myData) {
  JSONVar jsonDoc;
  jsonDoc["id"] = myData->ID;
  jsonDoc["isOpen"] = myData->isOpen;
  jsonDoc["codeFromUser"] = myData->codeFromUser;

  String jsonString = JSON.stringify(jsonDoc);
  WiFiClient client;
  HTTPClient http;
  String send_Data = serverName + "/unlock";

  http.begin(client, send_Data);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonString);

  String response = http.getString();
  
  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);
  Serial.print("Odpowiedź z serwera: ");
  Serial.println(response);

  http.end();
}

void getDevicesData(struct_message* myData, int id){
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
     HTTPClient http;

      String getData = serverName + "/devices/" + String(id);

     http.begin(client, getData.c_str());
     
       
     int httpResponseCode = http.GET();
     
     if (httpResponseCode>0) {
        String payload = http.getString();
        JSONVar jsonDoc;
        jsonDoc = JSON.stringify(payload);
        myData->ID=jsonDoc["id"];
        myData->isOpen=jsonDoc["isOpen"];
        myData->codeFromUser=jsonDoc["codeFromUser"];
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
}

//Helper routine to dump a byte array as hex values to Serial
void printHex(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

//Helper routine to dump a byte array as dec values to Serial
void printDec(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
    input_password[i] = buffer[i];
  }
}

void closeRemotely(struct_message *myData){
  if(!myData->isOpen){
    closes();
    
  }else if(myData->isOpen){
    opens();
  }
}

void check(struct_message *myData) {
  if (String(myData->codeFromUser) == input_password)
  {
    opens();
    delay(3000);
    
    if(!state){
      opens();
      sendUnlock(myData);
      state = 1;
      return;
    }
    closes();
    state = 0;
  }
}

void closes() {
  for (int i = 0; i <= 180; i++) {
    myservo.write(i);
    delay(10);
  }
}

void opens() {
  for (int i = 180; i >= 0; i--) {
    myservo.write(i);
    delay(10);
  }
}