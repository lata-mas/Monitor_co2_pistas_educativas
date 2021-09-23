#ifndef ARDUINO_ARCH_ESP8266
#error Requires NodeMCU esp8266 board
#endif 
#define DEBUG
/*
  monitorCO2.ino

  Optimized for NodeMCU ESP8266 dev board
    
  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>

  Supported sensors:

  * DUMMY
    A dummy sensor for test purposes
    
  * SEN0220
    DFRobot SEN0220. UART interface.

  * S8LP
    SenseAir S8 Low Power. UART interface.

  * SCD30
    Sensirion SCD30 sensor module. i2c (TWI) interface
*/
#define S8LP

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <TM1637Display.h>
#include "myEEPROM.h"
#include "arduino_secrets.h"
#include "cma.h"

#define MAX_READ_ERRORS 8
#define MAX_WIFI_ERRORS 8

#define CLK 14
#define DIO 12

#define POST_INTERVAL 60
#define POST_MINIMAL

#define LED_RED 16
#define LED_GREEN 15
#define LED_BLUE 13

#define CO2_MIN 400
#define CO2_MID 600
#define CO2_MAX 800

#define BUZZER 2

#define BUTTON_0  0

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 8192

const byte ledRGB[]={LED_RED, LED_GREEN, LED_BLUE};
const byte colorBlack[]={1,1,1}; // 1=OFF 0=ON
const byte colorGreen[]={1,0,1};
const byte colorYellow[]={0,0,1};
const byte colorRed[]={0,1,1};

/*
const char* ssid = SECRET_SSID;
const char* pass = SECRET_PASS;
*/

WiFiMode_t wifiMode;

myEEPROM myEEPROM;
eeprom_data_t *eeprom_data;

Ticker tickerSensor;
Ticker tickerPost;
Ticker tickerBuzzer;

TM1637Display display(CLK, DIO);

#define ERROR_WIFI_NOT_CONNECTED 0xf000
#define ERROR_SENSOR_READ 0xe000
#define ERROR_SENSOR_INIT 0xe001
#define ERROR_MEASUREMENT_INTERVAL 0xe002
#define ERROR_ASC_FAILED 0xe003

/*
 * Esta parte está incompleta
 */
#define LISTEN_PORT 80

ESP8266WebServer server(LISTEN_PORT);

/*
 * Fin de la parte incompleta
 */

#ifdef SCD30
/****************************************
 * Sensor SCD30 0-10K CO2 ppm            
 *   i2c addr = 0x61
 *   Thanks Lady Ada!
 ****************************************/
#include <Adafruit_SCD30.h>
#define MEASUREMENT_INTERVAL 2

Adafruit_SCD30 scd30;

short initSensor() {
  if(!scd30.begin()) {
#ifdef DEBUG
    Serial.println(F("Init scd30 failed!"));
#endif
    noReturn(ERROR_SENSOR_INIT);
  }
  delay(SHORT_DELAY);
  
  if(scd30.getMeasurementInterval()!=MEASUREMENT_INTERVAL) {
#ifdef DEBUG
    Serial.printf(PSTR("Interval=%d\n"), MEASUREMENT_INTERVAL);
#endif
    delay(SHORT_DELAY);
    if(!scd30.setMeasurementInterval(MEASUREMENT_INTERVAL)) noReturn(ERROR_MEASUREMENT_INTERVAL);
  }

  if(!scd30.selfCalibrationEnabled()) {
    delay(SHORT_DELAY);
    if(!scd30. selfCalibrationEnabled(true)) noReturn(ERROR_ASC_FAILED);
#ifdef DEBUG
    Serial.println(F("ASC enabled..."));
#endif
  }
  return 0;
}

short readSensor() {
  if (!scd30.dataReady()) return -1;
  if (!scd30.read()) return -1;
  return scd30.CO2;
}

#define SENSOR_NAME "scd30"
#endif

#if defined(SEN0220) || defined(MH_Z14A)
/****************************************
 * Sensor SEN0220 0-50K CO2 ppm            
 *   Serial1 requires Arduino Leonardo
 *   or similar board
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#include <SoftwareSerial.h>
#define RX_SW 5
#define TX_SW 4
#define MEASUREMENT_INTERVAL 2

SoftwareSerial SerialSW(RX_SW, TX_SW);

unsigned char hexdata[] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 

typedef struct {
  byte ff, cmd, hi, lo, fill[4], crc;
} data_sen0220_t;

short sen0220(void) {
  union {
    struct { byte lo, hi; };
    word w;
  } retval;
  data_sen0220_t data;
  SerialSW.write(hexdata,sizeof(hexdata));
  SerialSW.listen();
  delay(500); // TO BIG!
  if (SerialSW.available()>0) { 
    SerialSW.readBytes((char*)&data, 9);
    if((data.ff != 0xff) && (data.cmd != 0x86)) return -1;
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
  else return -1;    
}

unsigned char SET_RANGE_2K[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x07,0xD0,0x8F};

short initSensor() {
  SerialSW.begin(9600);
  delay(SHORT_DELAY);
  SerialSW.write(SET_RANGE_2K, sizeof(SET_RANGE_2K));
  delay(SHORT_DELAY);
  if(sen0220()==-1) return -1;
  else return 0;
}

short readSensor() {
  return sen0220();
}

#ifdef SEN0220
#define SENSOR_NAME "sen0220"
#elif defined(MH_Z14A)
#define SENSOR_NAME "mh-z14a"
#endif
#endif

#ifdef S8LP
/****************************************
 * SenseAir S8            
 *   UART interface
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#include <SoftwareSerial.h>
#define RX_SW 5
#define TX_SW 4
#define MEASUREMENT_INTERVAL 4

SoftwareSerial SerialS8(RX_SW, TX_SW);

static const byte S8_CMD_STATUS[]  = {0xfe, 0x04, 0x00, 0x00, 0x00, 0x01, 0x25, 0xc5};
static const byte S8_CMD_CO2READ[] = {0xfe, 0x04, 0x00, 0x03, 0x00, 0x01, 0xd5, 0xc5};

typedef struct {
  byte addr,
   function_code,
   count,
   hi,
   lo,
   crc_hi,
   crc_lo;
} s8data_t;

typedef union {
    struct {
      byte lo,hi;
    };
    word w;
  } retval_t;

#define S8_WAIT_DELAY 100

short s8status(void) {
  retval_t retval;
  s8data_t data;
  
  SerialS8.write((char*)S8_CMD_STATUS, sizeof(S8_CMD_STATUS));
  SerialS8.listen();
  delay(S8_WAIT_DELAY);
  if (SerialS8.available()>0)
    SerialS8.readBytes((char*)&data,sizeof(data));
  else return -1;
  
  if((data.addr != 0xfe) && (data.function_code != 0x04)) return -1;
  else {
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
}

short s8co2read(void) {
  retval_t retval;
  s8data_t data;
  
  SerialS8.write((char*)S8_CMD_CO2READ, sizeof(S8_CMD_CO2READ));
  SerialS8.listen();
  delay(S8_WAIT_DELAY);
  if (SerialS8.available()>0)
    SerialS8.readBytes((char*)&data,sizeof(data));
  else return -1;

  if((data.addr != 0xfe) && (data.function_code != 0x04)) return -1;
  else {
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
}

short initSensor() {
  SerialS8.begin(9600);
  delay(SHORT_DELAY);
  return s8status();
}

short readSensor() {
  return s8co2read();
}

#define SENSOR_NAME "s8lp"
#endif 

#ifdef DUMMY
/****************************************
 * Dummy Sensor
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#define MEASUREMENT_INTERVAL 5

static int myDummyData=450;

short initSensor() {
  static boolean st = true;
  if(st) {
    randomSeed(0xc0cac07a);
    st = false;
  } else random();
  if (millis()&1!=0) return 0;
  else return -1;
}

short readSensor() {
  switch(random(10)) {
    case 1: return -1; // read error
    case 2: return ++myDummyData;
    case 3: return --myDummyData;
    case 5: return myDummyData+=10;
    case 8: return myDummyData-=10;
    default: return myDummyData;
  }
}

#define SENSOR_NAME "dummy"
#endif

#ifndef SENSOR_NAME
#error "PLEASE SELECT A SENSOR"
#endif
/*********************
 * Main program here!
 *********************/

void displayError(unsigned short errorCode) {
  boolean led=false;      
  for(short d=0; d<DELAY; d+=SHORT_DELAY) {
    if(led=!led) display.showNumberHexEx(errorCode,0,true);
    else display.clear();
    delay(SHORT_DELAY);
  }
}

void noReturn(unsigned short errorCode) {
  for(boolean led=false;;delay(SHORT_DELAY)) {
    if(led=!led) display.showNumberHexEx(errorCode, 0, true);
    else display.clear();
  }
}

#ifdef _CMA_H_
#define SAMPLES (POST_INTERVAL/MEASUREMENT_INTERVAL)

CMA cmaCO2(SAMPLES);
#endif

void setup() {
  pinMode(BUTTON_0, INPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DIO, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);

#ifdef DEBUG
  Serial.begin(115200);
  delay(SHORT_DELAY);
#endif

#ifdef DEBUG
  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F(" Proyecto monitorCO2"));
  Serial.println(F(__DATE__  " " __TIME__));
  Serial.println(F(ARDUINO_BOARD));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println(F("(C) 2021, IER-UNAM"));
  Serial.println(F("(C) 2021, UNAM"));
  Serial.println(F("*****************************"));
  Serial.println();

  Serial.println(F("Sensor: " SENSOR_NAME));
#endif

  powerOnSelfTest();

#ifdef DEBUG
  Serial.println(F("Init..."));
#endif
  while(1) {
    short st=initSensor();
    if(st==0) break;
    displayError(ERROR_SENSOR_INIT);
  }

  String mDNSname = WiFi.macAddress().substring(9);
  mDNSname.toLowerCase();
  mDNSname.replace(":","");
  mDNSname = F("monitorCO2-") + mDNSname;
/*
 * Check EEPROM data
 */
  eeprom_data = new eeprom_data_t;

  if(myEEPROM.readData(eeprom_data)) {
#ifdef DEBUG
    Serial.println(F("OK EEPROM DATA, WIFI-STATION..."));
#endif
    WiFi.mode(wifiMode=WIFI_STA);
    WiFi.begin(eeprom_data->wifi_ssid, eeprom_data->wifi_pass);
    chkwifi(true);
  } else {
#ifdef DEBUG
    Serial.println(F("NO EEPROM DATA, WIFI-ACCESS-POINT..."));
#endif
    WiFi.mode(wifiMode=WIFI_AP);
    WiFi.softAP(mDNSname);
  }

#ifdef DEBUG
  Serial.print(F("mDNS:"));
  Serial.println(mDNSname);
#endif
  MDNS.begin(mDNSname);

#ifdef DEBUG
  Serial.println(F("Ticker..."));
#endif
  tickerSensor.attach(MEASUREMENT_INTERVAL, tckrRead);
  tickerPost.attach(POST_INTERVAL, tckrPost);
  tickerBuzzer.attach_ms(SHORT_DELAY, tckrBuzzer);

  /*
   * Esta sección está incompleta
   */
#ifdef DEBUG
  Serial.println(F("WebServer..."));
#endif
/*
 * WEB SERVER SETUP
 */
  server.on(F("/"), handleRoot);
  server.on(F("/commit"), HTTP_POST, handleCommit);
  server.on(F("/reset"), handleReset);
#ifdef SCD30
  server.on(F("/FRC"), handleFRC);
  server.on(F("/ASC"), handleASC);
#endif
  server.begin();
#ifdef DEBUG
  Serial.println("HTTP server started...");
#endif
  /*
   * Termina la sección incompleta
   */
#ifdef DEBUG
  Serial.println("leaving setup()");
#endif
}

volatile short co2ppm = -1;
volatile boolean isTime2read = true;
volatile boolean isTime2post = true;
volatile boolean isTime2buzz = false;

void loop(void) {
  if(isTime2buzz) { // FIRST FASTER
    doBuzzer();
    isTime2buzz=false;
  }
  if(isTime2read) { 
    doReadSensor();
    isTime2read=false;
  }
  if(isTime2post) { // LAST SLOWER
    doPost();
    isTime2post=false;
  }
  server.handleClient();
}

void tckrPost() { isTime2post = true; }
void tckrRead() { isTime2read = true; }

/*
 * doReadSensor
 * 
 * keep rgbLED state
 * trigger ALARM 
 */

void doReadSensor(void) {
  short x = readSensor();
#ifndef _CMA_H_
  co2ppm = x;
#endif
  static byte readErrors=0;
  if(x<=0) { // CHECK READ ERRORS
    readErrors++;
#ifdef DEBUG
    Serial.print("readSensor Error: ");
    Serial.println(readErrors);
#endif
    displayError(ERROR_SENSOR_READ);
    if(readErrors>MAX_READ_ERRORS) ESP.reset();
    return; 
  } else if(readErrors>0) readErrors--;
    
#ifdef _CMA_H_
  cmaCO2.addData(x);
  co2ppm = cmaCO2.avg();
#endif

#ifdef DEBUG
  Serial.print(F("CO2ppm="));
  Serial.println(co2ppm);
#endif
  display.showNumberDec(co2ppm);

  const byte *color=colorBlack;
  if(co2ppm>=CO2_MIN && co2ppm<CO2_MID) {
#ifdef DEBUG
    Serial.println("GREEN");
#endif
    color=colorGreen;
  }
  else if(co2ppm>=CO2_MID && co2ppm<CO2_MAX) {
#ifdef DEBUG
    Serial.println("YELLOW");
#endif
    color=colorYellow;
    startAlarm(4); // 4 STATE CHANGES, 2 BEEPS, 1/2 SECOND
  }
  else if(co2ppm>=CO2_MAX) {
#ifdef DEBUG
    Serial.println("RED");
#endif
    color=colorRed;
    startAlarm(6); // 6 STATE CHANGES, 3 BEEPS, 3/4 SECOND
  }
  for(byte i=0; i<3; ++i)
    digitalWrite(ledRGB[i], color[i]);
}

/*
 * BUZZER over TICKER
 */

volatile byte buzzerDuration = 0;

void startAlarm(byte duration) {
  if(duration==0) return;
  buzzerDuration=duration;
}

void tckrBuzzer(void) {
  if(buzzerDuration>0) {
    buzzerDuration--;
    isTime2buzz=true;
  }
}

void doBuzzer() {
  boolean state=(buzzerDuration&1)!=0; // ENABLE ON HIGH
#ifdef DEBUG
  Serial.printf(PSTR("Buzzer %s\n"), 
    state?PSTR("ON"):PSTR("OFF"));
#endif
  digitalWrite(BUZZER, state);
}

/*
 * POST/KEEP WiFi
 */

void doPost(void) {
  if(wifiMode==WIFI_AP) return;
  static byte wifiErrors=0;
  if(!WiFi.isConnected()) {
    if(++wifiErrors>MAX_WIFI_ERRORS) ESP.reset();
    WiFi.reconnect();
    chkwifi(false);
  } else if(wifiErrors>0) wifiErrors--;

  MDNS.update();
  
  WiFiClient wifiClient;
  HTTPClient httpClient;

  String payload = F("{");
  payload += F("\"" SENSOR_NAME "\":");
  payload += String(co2ppm);
#ifndef POST_MINIMAL
  payload += F(",\"heartbeat\":");
  payload += millis();
#ifdef SCD30
  payload += F(",\"temperature\":");
  payload += String(scd30.temperature, 1);
  payload += F(",\"relative_humidity\":");
  payload += String(scd30.relative_humidity, 0);
#endif
#endif
  payload += F("}");

  String URL=F(SECRET_URL_PREFIX) + eeprom_data->access_token + F(SECRET_URL_POSTFIX);
  
#ifdef DEBUG
  Serial.println(WiFi.localIP());
  Serial.print(F("POST: "));
  Serial.println(payload);
  Serial.println(URL);
#endif

  httpClient.begin(wifiClient, URL);
  httpClient.addHeader(F("Content-Type"), F("application/json"));

  int statusCode = httpClient.POST(payload);
  
#ifdef DEBUG
  Serial.print(F("ST="));
  Serial.print(statusCode);
  if(statusCode<0) {
    Serial.print(":");
    Serial.print(httpClient.errorToString(statusCode));
  }
  Serial.println();
#endif
}

void chkwifi(boolean retry) {
  int st;

  delay(SHORT_DELAY);
#ifdef DEBUG
  Serial.print(F("WiFi SSID: "));
  Serial.println(WiFi.SSID());
#endif

  while(!WiFi.isConnected()) {
#ifdef DEBUG
    st = WiFi.status();
    Serial.print("WiFi.status=");
    Serial.println(wl_status_to_string(st));
#endif
    displayError(ERROR_WIFI_NOT_CONNECTED+st);
    if(!retry) break;
    static byte i=1;
    if((i<<=1)==0) break;
  }
#ifdef DEBUG
  if(WiFi.isConnected()) {
    Serial.print("IP address=");
    Serial.println(WiFi.localIP());
  }
#endif
}

const char* wl_status_to_string(int status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
  }
  return NULL;
}

void powerOnSelfTest(void) {
  short holdButton=0;
#ifdef DEBUG
  Serial.println("Power On Self Test...");
#endif  
  while(1) {
#ifdef DEBUG
    Serial.println("DISPLAY=8888");
#endif
    display.setBrightness(0x07);
    display.showNumberDec(8888);

    for(byte led=0; led<3; led++) {
#ifdef DEBUG
      switch(led) {
        case 0: Serial.println("LED RED"); break;
        case 1: Serial.println("LED GREEN"); break;
        case 2: Serial.println("LED BLUE"); break;
      }
#endif
      digitalWrite(BUZZER, HIGH);
      digitalWrite(ledRGB[led], LOW);
      delay(SHORT_DELAY);
      digitalWrite(BUZZER, LOW);
      delay(SHORT_DELAY);
      digitalWrite(ledRGB[led], HIGH);
    }
    display.clear();
    delay(SHORT_DELAY);
    if(digitalRead(BUTTON_0)==HIGH) break;
    holdButton++;
#ifdef DEBUG
    Serial.println(holdButton);
#endif
  }
  if(holdButton>3) {
#ifdef DEBUG
    Serial.println(F("Erasing EEPROM...")); 
#endif
    myEEPROM.clear();
  }
}
/*
 * WEBSERVER
 * 
 * Esta parte está incompleta
 * 
 */
 
void handleRoot() {
#ifdef DEBUG
  Serial.println(F("handleRoot()"));
#endif
  server.send(200, F("text/html"), F("<!DOCTYPE html>"
"<html><body>"
"<h1>MonitorCO2</h1>"
"<p>Proporciona los siguientes datos:</p>"
"<form action=\"/commit\" method=\"post\">"
"<label for=\"ssid\">WiFi SSID:</label><br>"
"<input type=\"text\" id=\"ssid\" name=\"ssid\"><br>"
"<label for=\"pass\">WiFi password:</label><br>"
"<input type=\"text\" id=\"pass\" name=\"pass\"><br>"
"<label for=\"token\">Access Token:</label><br>"
"<input type=\"text\" id=\"token\" name=\"token\"><br>"
"<input type=\"submit\" value=\"Submit\">"
"</form>"
"</body></html>")
  );
}

void handleCommit() {
#ifdef DEBUG
  Serial.println(F("handleCommit()"));
#endif
  for (uint8_t i = 0; i < server.args(); i++) {
    if(server.argName(i)=="token") 
      eeprom_data->access_token=server.arg(i);
    if(server.argName(i)=="ssid") 
      eeprom_data->wifi_ssid=server.arg(i);
    if(server.argName(i)=="pass") 
      eeprom_data->wifi_pass=server.arg(i);
  }
  myEEPROM.writeData(eeprom_data);
  server.send(200, "text/html", "<!DOCTYPE html>"
"<html><body>"
"<h1>MonitorCO2</h1>"
"<p>Datos grabados, por favor, reinicia el monitorCO2</p>"
"</body></html>"
  );
}

const char* www_username = SECRET_WWW_USERNAME;
const char* www_password = SECRET_WWW_PASSWORD;

void handleReset() {
#ifdef DEBUG
  Serial.println(F("handleReset()"));
#endif
  if (!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
  myEEPROM.clear();
  server.send(200, "text/plain", "Reset OK");
}

#ifdef SCD30
void handleFRC(void) {
  if (!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
  scd30.forceRecalibrationWithReference(450);  // perhaps this is a parameter?
  server.send(200, "text/plain", "FRC started");
}

void handleASC(void) {
  if (!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
  scd30.selfCalibrationEnabled(true);
  server.send(200, "text/plain", "ASC started");
}
#endif

/*
 * Termina la parte incompleta
 */
 
