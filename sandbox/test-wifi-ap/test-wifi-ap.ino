#ifndef ARDUINO_ARCH_ESP8266
#error Requires NodeMCU esp8266 board
#endif 
#define DEBUG
/*
  proyecto monitorCO2

  prueba del EEPROM para grabar la configuración
  individual de cada sensor (sic)

  Si no hay datos en la EEPROM entra a modo WIFI_AP
  de los contrario entra a WIFI_STA
    
  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>
*/
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include "myEEPROM.h"

#define SHORT_DELAY 128
#define DELAY 1024
#define LONG_DELAY 8192

myEEPROM myEEPROM;
eeprom_data_t *data;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  randomSeed(0xC0CAC07A);
  Serial.println("EEPROM test with WIFI-AP");
  data = new eeprom_data_t;

  String mDNSname = WiFi.macAddress().substring(9);
  mDNSname.toLowerCase();
  mDNSname.replace(":","");
  mDNSname = F("monitorCO2-") + mDNSname;

  IPAddress myIP;

  if(myEEPROM.readData(data)) {
    Serial.println("OK EEPROM DATA, WIFI-STATION...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(data->wifi_ssid, data->wifi_pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(DELAY);
      Serial.print(".");
    }
    Serial.println();
    myIP=WiFi.localIP();
  } else {
    Serial.println("NO EEPROM DATA, WIFI-ACCESS-POINT...");
    WiFi.softAP(mDNSname);
    Serial.println(mDNSname);
    myIP=WiFi.softAPIP();
  }
  Serial.print("IPaddres=");
  Serial.println(myIP);

#ifdef DEBUG
  Serial.print("mDNS:");
  Serial.println(mDNSname);
#endif
  MDNS.begin(mDNSname);
/*
 * WEB SERVER SETUP
 */
  server.on(F("/"), handleRoot);
//  server.on(F("/commit"), HTTP_POST, handleCommit);
  server.on(F("/commit"), handleCommit); // TEST WITH GET
  server.on(F("/reset"), handleReset);
  server.begin();
  Serial.println("HTTP server started...");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  Serial.println("handleRoot()");
  server.send(200, "text/plain", "Root OK");
}

void handleCommit() {
  Serial.println("handleCommit()");
  server.send(200, "text/plain", "Commit OK");
  data->access_token="nCsfwwiNH8JpXLmI2yuY";
  data->wifi_ssid="IER";
  data->wifi_pass="acadier2014";
  myEEPROM.writeData(data);
}

void handleReset() {
  Serial.println("handleReset()");
  server.send(200, "text/plain", "Commit OK");
  myEEPROM.clear();
}
