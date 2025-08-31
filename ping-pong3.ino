#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <Adafruit_SleepyDog.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"


char serverAddress[]  = "ge4zhwq3bg.execute-api.ap-northeast-1.amazonaws.com";
char endPoint[]       = "/default/smsSend";
int port              = 443;
char contentType[]    = "application/json";

char *bodyTelNos[]      = {"+819091893525","+819085518831"};
//char *bodyTelNos[]  = {"+819091893525"};
char bodySenderId[] = "PINGPONG";
char bodyMessage[]  = "ぴんぽ～ん、おうちのチャイムが鳴りました！";

int channel              = 0;
int threshold            = 300;
unsigned long interval   = 200;
unsigned long idle_time  = 4000;
//unsigned long reset_time = 3600000;
int watchdog_time        = 8000;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status                     = WL_IDLE_STATUS;
int volume                     = 0;
unsigned long current_time     = 0;
unsigned long last_detect_time = 0;
int volume_offset              = 512;

WiFiSSLClient wifi;
HttpClient client   = HttpClient(wifi, serverAddress, port);
char apikey[]       = API_KEY;

JsonDocument postDataJson;
String postData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  //snprintf(postData, sizeof(postData), "telnos=%s&senderid=\"%s\"&message=\"%s\"", bodyTelNos, bodySenderId, bodyMessage);
  JsonArray telnos = postDataJson["telnos"].to<JsonArray>();
  for(char* telno : bodyTelNos) {
    telnos.add(telno);
  }
  postDataJson["senderid"]  = bodySenderId;
  postDataJson["message"]   = bodyMessage;
  serializeJson(postDataJson, postData);

  int countdownMS = Watchdog.enable(watchdog_time);
  Serial.print("Enabled the watchdog with max countdown of ");
  Serial.print(countdownMS, DEC);
  Serial.println(" ms!");
}

//void(* resetFunc) (void) = 0;

void loop() {
  // put your main code here, to run repeatedly:
  current_time = millis();
  digitalWrite(LED_BUILTIN, LOW);

  //if(current_time > reset_time) { 
  //  Serial.print("Reset cycle has come. :");
  //  Serial.println(current_time);
  //  Watchdog.reset(); 
  //}

  volume = analogRead(channel) - volume_offset;

  //Serial.print(current_time);
  //Serial.print(" : ");
  //Serial.println(volume);

  if(abs(volume) > threshold) {
    if(current_time - last_detect_time > idle_time){
      Serial.println("Ping-Pong detected.");
      digitalWrite(LED_BUILTIN, HIGH);

      Serial.print("Message body: ");
      Serial.println(postData);

      client.beginRequest();
      client.post(endPoint);
      client.sendHeader("x-api-key", apikey);
      client.sendHeader(contentType);
      client.sendHeader("Content-Length", postData.length());
      client.beginBody();
      client.print(postData);
      client.endRequest();

      //client.get(endPoint);

      Serial.println("Request sent.");
      
      int statusCode = client.responseStatusCode();
      Serial.print("Status code: ");
      Serial.println(statusCode);

      String response = client.responseBody();
      Serial.print("Response: ");
      Serial.println(response);

      last_detect_time = current_time;
    }
  }
  
  Watchdog.reset(); 
  delay(interval);
}