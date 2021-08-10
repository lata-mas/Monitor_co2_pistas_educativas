#include <ArduinoHttpClient.h>
#include <ESP8266WiFi.h>
#include "TM1637Display.h"

#include "SoftwareSerial.h"
SoftwareSerial mySerial(13, 15); // RX, TX
unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; //Read the gas density command /Don't change the order

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SECRET_SERVER, SECRET_PORT);

int status = WL_IDLE_STATUS;
TM1637Display displayseg(14,12);

int i = 0;

void setup() {

  displayseg.setBrightness(7,true);
  displayseg.clear();
  
  displayseg.showNumberDec(1234,false,4,0);
  WiFi.begin(ssid, pass);
  InitWiFi();
  

  Serial.begin(9600);
  while (!Serial) {

  }
  mySerial.begin(9600);

   displayseg.clear();

}

void loop() {
   mySerial.write(hexdata,9);
   delay(500);

 for(int i=0,j=0;i<9;i++)
 {
  if (mySerial.available()>0)
  {
     long hi,lo,CO2;
     int ch=mySerial.read();

    if(i==2){     hi=ch;   }   //High concentration
    if(i==3){     lo=ch;   }   //Low concentration
    if(i==8) {
               CO2=hi*256+lo;  //CO2 concentration
      Serial.print("CO2 concentration: ");
      Serial.print(CO2);
      Serial.println("ppm");

     int co2sen = CO2;
     displayseg.showNumberDec(co2sen,false,4,0);
 
    String payload = "{";
    payload += "'sen0220':";
    payload += co2sen;
    payload += "}";

    Serial.println("POST... " + payload);

    
    httpClient.post(SECRET_TOPIC, "application/json", payload);

    int statusCode = httpClient.responseStatusCode();  
    String response = httpClient.responseBody();
    Serial.print("Status code: ");
    Serial.print(statusCode);
    Serial.print(" ");
    Serial.println(response);
    delay(5000);
               }

  }

 }

 }

 void InitWiFi()
{
   Serial.println("Connecting to AP ...");
   Serial.println(ssid);
  // attempt to connect to WiFi network

  WiFi.begin(ssid, pass);
  while (WiFi.status() == WL_CONNECTED) {
    delay(500);
    Serial.println("Connected to AP");
  } 
}

void reconnect() {
  
  // Loop until we're reconnected
 // Loop until we're reconnected
     if  ( i < 5) {
      Serial.println("Reintentando conexion...");
      WiFi.begin(ssid, pass);
      delay(5000);
     // esp_task_wdt_reset();
      i++;
      if (i == 5) {
        Serial.println("Conexion Fallida, reset en 3 seg");
       // esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
        //esp_task_wdt_add(NULL); //add current thread to WDT watch
      }
     }
      
}
