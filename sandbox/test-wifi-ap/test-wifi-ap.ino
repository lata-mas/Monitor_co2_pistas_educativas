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
#include "arduino_secrets.h"

#define SHORT_DELAY 128
#define DELAY 1024
#define LONG_DELAY 8192

myEEPROM myEEPROM;
eeprom_data_t *eeprom_data;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  Serial.println("EEPROM test with WIFI-AP");
  eeprom_data = new eeprom_data_t;

  String mDNSname = WiFi.macAddress().substring(9);
  mDNSname.toLowerCase();
  mDNSname.replace(":","");
  mDNSname = F("monitorCO2-") + mDNSname;

  IPAddress myIP;

  if(myEEPROM.readData(eeprom_data)) {
    Serial.println("OK EEPROM DATA, WIFI-STATION...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(eeprom_data->wifi_ssid, eeprom_data->wifi_pass);
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
    WiFi.mode(WIFI_AP);
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
  server.on(F("/commit"), HTTP_POST, handleCommit);
  server.on(F("/reset"), handleReset);
  server.begin();
  Serial.println("HTTP server started...");
}

void loop() {
  server.handleClient();
  MDNS.update();
}

void handleRoot() {
  Serial.println("handleRoot()");
  server.send(200, "text/html", "<!DOCTYPE html>"
"<html><body>"
"<h1>MonitorCO2</h1>"
"<p>Proporciona los siguientes datos:</p>"
"<form action=\"/commit\" method=\"post\">"
"<label for=\"token\">Access Token:</label><br>"
"<input type=\"text\" id=\"token\" name=\"token\"><br>"
"<input type=\"submit\" value=\"Submit\">"
"</form>"
"</body></html>"
  );
}

void handleCommit() {
  Serial.println("handleCommit()");
  for (uint8_t i = 0; i < server.args(); i++) {
    if(server.argName(i)=="token") 
      eeprom_data->access_token=server.arg(i);
  }
  eeprom_data->wifi_ssid=SECRET_SSID;
  eeprom_data->wifi_pass=SECRET_PASS;
  myEEPROM.writeData(eeprom_data);
  server.send(200, "text/html", "<!DOCTYPE html>"
"<html><body>"
"<h1>MonitorCO2</h1>"
"<p>Datos grabados, por favor, reinicia el monitorCO2</p>"
"</body></html>"
  );
}

void handleReset() {
  Serial.println("handleReset()");
  myEEPROM.clear();
  server.send(200, "text/plain", "Reset OK");
}
