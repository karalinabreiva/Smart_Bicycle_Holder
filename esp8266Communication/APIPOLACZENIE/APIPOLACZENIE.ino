
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
typedef struct struct_message {
  int ID;
  bool isOpen;
  int codeFromUser;
  } struct_message;

struct_message myData;
//const char* ssid = "iPhone (Adam)";
//const char* password = "hejkanaklejka";
const char* ssid = "PLAY_Swiatlowodowy_12E2";
const char* password = "4fE7vqetqVe#";


String serverName = "http://192.168.0.37:3001";

unsigned long lastTime = 0;

unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(9600); 
  myData.ID = 0;
  myData.isOpen = false;
  myData.codeFromUser = 1234;
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
    sendJSONData(&myData);
    delay(3000);
    getJSONData(&myData);
    delay(3000);
  
      
    
//    Serial.println("ID:");
//    Serial.println(myData.ID);
//    Serial.println("isOpen:");
//    Serial.println(myData.isOpen);
//    Serial.println("code:");
//    Serial.println(myData.codeFromUser);
  }
