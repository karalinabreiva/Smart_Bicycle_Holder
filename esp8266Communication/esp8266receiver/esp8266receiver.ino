#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
/*
BEZ_NAPISOW, parent, COM4
MAC_ADDRESS: {0xC8, 0xC9, 0xA3, 0x06, 0x52, 0xE7};
*/
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int ID;
  bool isOpen;
  int codeFromUser;
  } struct_message;
const char* ssid = "PLAY_Swiatlowodowy_12E2";
const char* password = "4fE7vqetqVe#";
//const char* ssid = "iPhone (Adam)";
//const char* password = "hejkanaklejka";
String serverName = "http://192.168.0.37:3001";

struct_message myData;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;


uint8_t broadcastAddress1[] = {0xC8,0xC9,0xA3,0x09,0xE8,0x3F};//CING_CIONG
uint8_t broadcastAddress0[] = {0xCC,0x50,0xE3,0xC7,0x3A,0x10};//LOLIN
String input = "";
char code[4];
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("ID: ");
  Serial.println(myData.ID);
  Serial.print("BOOLEAN: ");
  Serial.println(myData.isOpen);
  Serial.print("CODE: ");
  Serial.println(myData.codeFromUser);
  Serial.println();
  sendJSONData(&myData);
}
 
void setup() {
  Serial.begin(9600);
  
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  
  
  esp_now_add_peer(broadcastAddress0, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_add_peer(broadcastAddress1, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_recv_cb(OnDataRecv);
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
}

void getJSONData(struct_message* myData){
  
    
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      String getData = serverName + "/getData";

      http.begin(client, getData.c_str());
      
        
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        JSONVar jsonDoc;
        jsonDoc = JSON.stringify(payload);
        myData->ID=jsonDoc["ID"];
        myData->isOpen=jsonDoc["isOpen"];
        myData->codeFromUser=jsonDoc["codeFromUser"];
      
      
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
   
}

void sendJSONData(struct_message* myData) {

  JSONVar jsonDoc;
  jsonDoc["ID"] = myData->ID;
  jsonDoc["isOpen"] = myData->isOpen;
  jsonDoc["codeFromUser"] = myData->codeFromUser;


  String jsonString = JSON.stringify(jsonDoc);

      WiFiClient client;


  HTTPClient http;

  String send_Data = serverName + "/sendData";

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

void loop() {

    getJSONData(&myData);
    delay(2000);
 
// if(Serial.available()){
//   input = Serial.readString();
//   input.trim();
//   int j=0;
//   for(int i=2;i<6;i++){
//    code[j] = input[i];
//    j++;
//   }
//   if(input[0] == '1'){
//    myData.ID = 1;
//    if(input[1] == '1'){
//      myData.isOpen = true;
//      }
//    else if(input[1] == '0'){
//      myData.isOpen = false;
//      }   
//      myData.codeFromUser = atoi(code); 
//      Serial.println(code); 
//      esp_now_send(broadcastAddress1, (uint8_t *) &myData, sizeof(myData));
//     
//   }
//   else if(input[0] == '0'){
//    myData.ID = 0;
//    if(input[1] == '1'){
//      myData.isOpen = true;
//      }
//    else if(input[1] == '0'){
//      myData.isOpen = false;
//      }

      esp_now_send(broadcastAddress0, (uint8_t *) &myData, sizeof(myData));
   //}
 }


 
}
