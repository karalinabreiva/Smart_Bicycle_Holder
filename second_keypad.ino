//--------------------------------------------------------------------------------------------
//                                         LIBRARIES
//--------------------------------------------------------------------------------------------
#include <Wire.h>
#include <Keypad.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>


//--------------------------------------------------------------------------------------------
//                                         VARIABLES
//--------------------------------------------------------------------------------------------
const int ROW_NUM = 4;
const int COLUMN_NUM = 3;

typedef struct struct_message {
  int ID;
  bool isOpen;
  int codeFromUser;
  } struct_message;
const char* ssid = "PLAY_Swiatlowodowy_12E2";
const char* password = "4fE7vqetqVe#";
String serverName = "http://192.168.0.37:3001";

struct_message myData;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pin_rows[ROW_NUM] = {16, 5, 4, 0};
byte pin_column[COLUMN_NUM] = {2, 14, 12};
const int servo_pin = 13;

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
String input_password;

Servo myservo;
int state = 1;


//--------------------------------------------------------------------------------------------
//                                         SETUP (RUN ONCE)
//--------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  input_password.reserve(4);
  myservo.attach(servo_pin);
  myservo.write(0);

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
  
  input_password.reserve(4);
  myservo.attach(servo_pin);
  myservo.write(0);
}


//--------------------------------------------------------------------------------------------
//                                         MAIN LOOP (RUN REPEATEDLY)
//--------------------------------------------------------------------------------------------
void loop(){
  getDevicesData(&myData,0);
  char key = keypad.getKey();

  if (key){
    Serial.println(key);
    if(key == '*') {
      input_password = ""; // clear input password
    } else if(key == '#') {
      if(String(myData.codeFromUser) == input_password) {
        Serial.println("password is correct");
        check(&myData);
      } else {
        Serial.println("password is incorrect, try again");
      }
      input_password = "";
    } else {
      input_password += key;
    }
  }
}


//--------------------------------------------------------------------------------------------
//                                         FUNCTIONS
//--------------------------------------------------------------------------------------------
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

void check(struct_message* myData){
    openServo();
    delay(3000);
    
    if(!state){
      openServo();
      sendUnlock(myData);
      state = 1;
      return;
    }
    closeServo();
    state = 0;
}

void closeRemotely(struct_message *myData){
  if(!myData->isOpen){
    closeServo();
    
  }else if(myData->isOpen){
    openServo();
  }
}

void closeServo() {
  for (int i = 0; i <= 180; i++) {
    myservo.write(i);
    delay(10);
  }
}

void openServo() {
  for (int i = 180; i >= 0; i--) {
    myservo.write(i);
    delay(10);
  }
}
