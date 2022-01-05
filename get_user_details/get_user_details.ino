#include <EEPROM.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
//#include <WiFiManager.h>
#include <ESPAsyncWiFiManager.h>
#include <PubSubClient.h>

// Library for mail
#include "NodeMcu_ESP8266_Gmail_Sender_by_FMT.h"
#pragma region Globals
#pragma endregion Globals

#define EEPROM_SIZE 100

AsyncWebServer server(80);
DNSServer dns;

int gotDetails = 0;

// MQTT initialization
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// User coin preference
const char* TOPICS[10] = {"14/BTC", "14/ETH", "14/LTC", "14/ADA", "14/DOT", "14/BCH", "14/XLM", "14/DOGE", "14/BNB", "14/XMR"};

// Variables for mailing
String recipientID;
String subject;
String body;

// MQTT Message
String message;

String inc = " % increase";
String dec = " % decrease";

const char* NAME_REF = "name";
String NAME = "";
const char* EMAIL_REF = "emailAddress";
String EMAIL = "";

const char* COINS[10] = {"BTC", "ETH", "LTC", "ADA", "DOT", "BCH", "XLM", "DOGE", "BNB", "XMR"};

int preferences[10] = {0,0,0,0,0,0,0,0,0,0};

const int pref_address = 5;
const int got_address = 3;
const int name_address = 60;
const int email_address = 30;

const int SLEEP_TIME = 10e6;

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<style>

html { 
  font-family: Helvetica; 
    display: inline-block; 
    margin: 0px auto; 
    }
    
input[type=text], select {
  width: 70%;
  padding: 12px 20px;
  margin: 8px 0;
  display: inline-block;
  border: 1px solid #ccc;
  border-radius: 4px;
  box-sizing: border-box;
}


input[type=submit] {
  width: 70%;
  background-color: #4CAF50;
  color: white;
  padding: 14px 20px;
  margin: 8px 0;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}

input[type=submit]:hover {
  background-color: #45a049;
}

div {
  border-radius: 5px;
  background-color: #f2f2f2;
  padding: 20px;
}
</style>

<body>

<h1>Crypto Care Configuration</h1>

<form action="/get">
  <label for="name">Name:</label><br>
  <input type="text" id="name" name="name"><br>

  <label for="emailAddress">Email Address:</label><br>
  <input type="text" id="emailAddress" name="emailAddress"><br>
  
  <input type="checkbox" id="BTC" name="BTC" value="BTC" class="checkbox">
  <label for="BTC"> Bitcoin</label><br>
  <input type="checkbox" id="ETH" name="ETH" value="ETH" class="checkbox">
  <label for="ETH"> Ethereum</label><br>
  <input type="checkbox" id="LTC" name="LTC" value="LTC" class="checkbox">
  <label for="LTC"> Litecoin</label><br>
  <input type="checkbox" id="ADA" name="ADA" value="ADA" class="checkbox">
  <label for="ADA"> Cardano</label><br>
  <input type="checkbox" id="DOT" name="DOT" value="DOT" class="checkbox">
  <label for="DOT"> Polcadot</label><br>
  <input type="checkbox" id="BCH" name="BCH" value="BCH" class="checkbox">
  <label for="BCH"> Bitcoin Cash</label><br>
  <input type="checkbox" id="XLM" name="XLM" value="XLM" class="checkbox">
  <label for="XLM"> Steller</label><br>
    <input type="checkbox" id="DOGE" name="DOGE" value="DOGE" class="checkbox">
  <label for="DOGE"> Dogecoin</label><br>
    <input type="checkbox" id="BNB" name="BNB" value="BNB" class="checkbox">
  <label for="BNB"> Binance Coin</label><br>
    <input type="checkbox" id="XMR" name="XMR" value="XMR" class="checkbox">
  <label for="XMR"> Monero</label><br><br>
  
  <input type="submit" value="Submit">
</form>

</body>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  
  AsyncWiFiManager wifiManager(&server,&dns);
  Serial.println("connecting.............");
  //WiFi.disconnect();
  wifiManager.autoConnect("InstantAP", "Hello");
  Serial.println("InstantAP created");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void sever_start(){
  Serial.println("Trying to connect with web server");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String Message;
    if (request->hasParam(NAME_REF)) {
      Message = request->getParam(NAME_REF)->value();
      NAME = Message;

      Serial.println("Writing Username\n");
      writeStringToEEPROM(name_address, NAME);

      Serial.println("Got Username\n");

      if (request->hasParam(EMAIL_REF)) {
        Message = request->getParam(EMAIL_REF)->value();
        EMAIL = Message;

        Serial.println("Getting Email\n");
        
        writeStringToEEPROM(email_address, EMAIL);

        Serial.println("Got Email\n");
        
        for (int i=0; i< 10; i++) {
          if (request->hasParam(COINS[i])) {
             Message = request->getParam(COINS[i])->value();
             preferences[i] = 1;
          } else {
          preferences[i] = 0;
            }
        }
        request->send(200, "text/html", "Details Updated Successfully."
                           "<br>You will receive notifications Through Your Mobile");
        for (int i=0; i<10;i++){
           Serial.println(preferences[i]);
        }

        Serial.println("Getting Preferences\n");
        writeIntArrayIntoEEPROM(pref_address, preferences, 10);
        Serial.println("Got Preferences\n");
        
        sendUserDetails();
        gotDetails = 1;
        Serial.println("Got user details");
        setGotDetails();
        //Serial.println(preferences);
        
        server.end();
      } 
      else {
        request->send(200, "text/html", "Please Enter a Email Address"
                           "<br><a href=\"/\">Return to Home Page</a>");
      }
    }
    else {
      request->send(200, "text/html", "Please Enter a Name "
                           "<br><a href=\"/\">Return to Home Page</a>");
    }
  });
  server.onNotFound(notFound);
  server.begin();
}

void setup() {

  EEPROM.begin(EEPROM_SIZE);

  esp_sleep_enable_timer_wakeup(SLEEP_TIME);
  getGotDetails();

  Serial.begin(115200);
  
  setup_wifi();
  if (!gotDetails){
    sever_start();
  }

  while (!gotDetails) {
     Serial.println(".");
     delay(100);
  }

  NAME = readStringFromEEPROM(name_address);
  EMAIL = readStringFromEEPROM(email_address);
  readIntArrayFromEEPROM (pref_address, preferences, 10);

  Serial.println(EMAIL);
  Serial.println("\n");
  Serial.println(NAME);
  Serial.println("\n");

  //Serial.println(preferences);

  MQTTSetup();

}

void loop() {
  client.loop();
}

//Method to Send User Details to Firebase
void sendUserDetails() {

        HTTPClient http;
        
        http.begin("https://crypto-care-ec4d9-default-rtdb.firebaseio.com/users.json");
        http.addHeader("Content-Type", "application/json");

        String JSONmessage = "{\"name\":\"";
        JSONmessage = JSONmessage + NAME;
        JSONmessage = JSONmessage + "\", \"preferences\":\"";
        
        String prefer = "";
        
        for (int i=0; i<10; i++){
          if (preferences[i] == 1){
            prefer = prefer + COINS[i];
            prefer = prefer + ",";            
          }
        }

        JSONmessage = JSONmessage + prefer;
        JSONmessage = JSONmessage + "\"}"; 
        int httpCode = http.POST(JSONmessage);
        String payload = http.getString();

        Serial.println(payload);
        Serial.println("\n");
        Serial.println(httpCode);
        Serial.println("\n");
        
        http.end();
}

void writeIntArrayIntoEEPROM(int address, int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++) 
  {
    EEPROM.write(addressIndex, numbers[i] >> 8);
    EEPROM.commit();
    EEPROM.write(addressIndex + 1, numbers[i] & 0xFF);
    addressIndex += 2;
    EEPROM.commit();
  }
}
void readIntArrayFromEEPROM(int address, int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++)
  {
    numbers[i] = (EEPROM.read(addressIndex) << 8) + EEPROM.read(addressIndex + 1);
    addressIndex += 2;
  }
}

void setGotDetails() {
  EEPROM.write(got_address, 1);
  EEPROM.commit();
}
void getGotDetails() {
  gotDetails = EEPROM.read(got_address);
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  int len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  byte newStrLen = EEPROM.read(addrOffset);
  Serial.println (newStrLen);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  return String(data);
}

void MQTTSetup() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(MQTTcallback);
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266"))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  // Subscribe user preference topic
  int i;
  for(i=0; i<10; i++) {
      if (preferences[i] == 1) {
        Serial.println(TOPICS[i]);
        client.subscribe(TOPICS[i]);
      }
  }
  
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) 
{
  message = "";
  Serial.print("\nMessage received in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  
  for (int i = 0; i < length; i++) 
  {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  Mailing(message);

  delay(1000);

  Serial.println("I'm awake, but I'm going into deep sleep mode for  seconds");
  //ESP.deepSleep(SLEEP_TIME);
  esp_deep_sleep_start();
}

void Mailing(String message) {
  Gsender *gsender = Gsender::Instance();
    
  recipientID = EMAIL; // enter the email ID of the recipient
  subject = "Hi! I am Cryptocurrency Advisor"; // enter the subject of the email
  body =  message;  // enter the body of the email
  
  if(gsender->Subject(subject)->Send(recipientID, body)) { // sends the email using a single line function
      Serial.println("Email sent"); // message confirmation
  } 
  else {
      Serial.print("Error sending message: ");
      Serial.println(gsender->getError()); // prints out the exact error if the email wasn't successfully sent
  }
}
