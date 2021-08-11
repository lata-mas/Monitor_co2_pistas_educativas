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
#define DUMMY

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <TM1637Display.h>
#include "arduino_secrets.h"

#define CLK 14
#define DIO 12

#define POST_INTERVAL 60
#define POST_MINIMAL

#define LED_RED 16
#define LED_GREEN 13
#define LED_BLUE 15

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

const char* ssid = SECRET_SSID;
const char* pass = SECRET_PASS;

Ticker tickerSensor;
Ticker tickerPost;
Ticker tickerBuzzer;

TM1637Display display(CLK, DIO);

#define ERROR_WIFI_NOT_CONNECTED 0xf000
#define ERROR_SENSOR_INIT 0xe001
#define ERROR_MEASUREMENT_INTERVAL 0xe002
#define ERROR_ASC_FAILED 0xe003

/*
 * Esta parte está incompleta
 */
#define LISTEN_PORT 80

ESP8266WebServer server(LISTEN_PORT);

const char *www_username = SECRET_WWW_USERNAME;
const char *www_password = SECRET_WWW_PASSWORD;
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
    Serial.println("ASC enabled...");
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
  delay(500);
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
#define MEASUREMENT_INTERVAL 1

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
    case 2: return ++myDummyData;
    case 3: return --myDummyData;
    case 5: return myDummyData+=10;
    case 7: return myDummyData-=10;
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

void displayError(short errorCode) {
  boolean led=false;      
  for(short d=0; d<DELAY; d+=SHORT_DELAY) {
    if(led=!led) display.showNumberHexEx(errorCode,0,true);
    else display.clear();
    delay(SHORT_DELAY);
  }
}

void noReturn(short errorCode) {
  for(boolean led=false;;delay(SHORT_DELAY)) {
    if(led=!led) display.showNumberHexEx(errorCode, 0, true);
    else display.clear();
  }
}

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

  display.setBrightness(0x07);
  display.showNumberDec(8888);

#ifdef DEBUG
  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F(" Proyecto monitorCO2"));
  Serial.println(F(__DATE__  " " __TIME__));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println(F("(C) 2021, IER-UNAM"));
  Serial.println(F("(C) 2021, UNAM"));
  Serial.println(F("*****************************"));
  Serial.println();

  Serial.println(F("Sensor: " SENSOR_NAME));
#endif
  delay(LONG_DELAY);

#ifdef DEBUG
  Serial.println(F("Init..."));
#endif
  while(1) {
    short st=initSensor();
    if(st==0) break;
    displayError(ERROR_SENSOR_INIT);
  }
  
  WiFi.mode(WIFI_STA);
  chkwifi(true);

  String mDNSname = WiFi.macAddress().substring(9);
  mDNSname.toLowerCase();
  mDNSname.replace(":","");
  mDNSname = F("monitorCO2-") + mDNSname;

#ifdef DEBUG
  Serial.print("mDNS:");
  Serial.println(mDNSname);
#endif
  MDNS.begin(mDNSname);

  /*
   * Esta sección está incompleta
   */
#ifdef DEBUG
  Serial.println(F("WebServer..."));
#endif
  server.on(F("/"), handleRoot);
  server.on(F("/commit/"), HTTP_POST, handleCommit);
  server.on(F("/frc/"), handleFRC);
  server.on(F("/asc/"), handleASC);
  
  server.begin();
  /*
   * Termina la sección incompleta
   */

#ifdef DEBUG
  Serial.println("Ticker...");
#endif
  tickerSensor.attach(MEASUREMENT_INTERVAL, tckrRead);
  tickerPost.attach(POST_INTERVAL, tckrPost);
  tickerBuzzer.attach_ms(SHORT_DELAY, tckrBuzzer);

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
  co2ppm = readSensor();  

#ifdef DEBUG
  Serial.print(F("CO2ppm="));
  Serial.println(co2ppm);
#endif
  display.showNumberDec(co2ppm);

  const byte *color=colorBlack;
  if(co2ppm>=400 && co2ppm<600) {
#ifdef DEBUG
    Serial.println("GREEN");
#endif
    color=colorGreen;
  }
  else if(co2ppm>=600 && co2ppm<750) {
#ifdef DEBUG
    Serial.println("YELLOW");
#endif
    color=colorYellow;
    startAlarm(4); // 4 STATE CHANGES, TWO BEEPS, HALF SECOND
  }
  else if(co2ppm>=750) {
#ifdef DEBUG
    Serial.println("RED");
#endif
    color=colorRed;
    startAlarm(8); // 8 STATE CHANGES, FOUR BEEPS, ONE SECOND
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
  if(WiFi.status()!=WL_CONNECTED) chkwifi(false);

  MDNS.update();
  
  WiFiClient wifiClient;
  HTTPClient httpClient;

  String payload = "{";
  payload += "\"" SENSOR_NAME "\":";
  payload += co2ppm;
#ifndef POST_MINIMAL
  payload += ",\"heartbeat\":";
  payload += millis();
#ifdef SCD30
  payload += ",\"temperature\":";
  payload += String(scd30.temperature, 1);
  payload += ",\"relative_humidity\":";
  payload += String(scd30.relative_humidity, 0);
#endif
#endif
  payload += "}";
  
#ifdef DEBUG
  Serial.print(F("POST: "));
  Serial.println(payload);
  Serial.println(PSTR(SECRET_URL));
#endif

  httpClient.begin(wifiClient, PSTR(SECRET_URL));
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

#ifdef DEBUG
  Serial.print(F("WiFi SSID: "));
  Serial.println(ssid);
#endif

  WiFi.begin(ssid,pass);

  while((st = WiFi.status()) != WL_CONNECTED) {
#ifdef DEBUG
    Serial.print("WiFi.status=");
    Serial.println(wl_status_to_string(st));
#endif
    displayError(ERROR_WIFI_NOT_CONNECTED+st);
    if(!retry) break;
  }
#ifdef DEBUG
  if(st == WL_CONNECTED) {
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

/*
 * WEBSERVER
 * 
 * Esta parte está incompleta
 * 
 */
 
void handleRoot(void) {
  if (!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
  server.send(200, "text/plain", "Login OK");
}

void handleCommit(void) {
  if (!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
  server.send(200, "text/plain", "Commit OK");
}

void handleFRC(void) {
  if (!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
#ifdef SCD30
  scd30.forceRecalibrationWithReference(450);
#endif
  server.send(200, "text/plain", "FRC started");
}

void handleASC(void) {
  if (!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
#ifdef SCD30
  scd30.selfCalibrationEnabled(true);
#endif
  server.send(200, "text/plain", "ABC started");
}

/*
 * Termina la parte incompleta
 */
 
