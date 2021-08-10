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
    DFRobot SEN0220.

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
#define LISTEN_PORT 80

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 8192

const char* ssid = SECRET_SSID;
const char* pass = SECRET_PASS;

const char *www_username = SECRET_WWW_USERNAME;
const char *www_password = SECRET_WWW_PASSWORD;

Ticker tickerSensor;
Ticker tickerPost;

TM1637Display display(CLK, DIO);

ESP8266WebServer server(LISTEN_PORT);

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
    Serial.println(F("Init scd30 failed!"));
    noReturn(0xe0f0);
  }
  delay(SHORT_DELAY);
  
  if(scd30.getMeasurementInterval()!=MEASUREMENT_INTERVAL) {
    Serial.printf(PSTR("Interval=%d\n"), MEASUREMENT_INTERVAL);
    delay(SHORT_DELAY);
    if(!scd30.setMeasurementInterval(MEASUREMENT_INTERVAL)) noReturn(0xe0f1);
  }

  if(!scd30.selfCalibrationEnabled()) {
    Serial.println("ABC...");
    delay(SHORT_DELAY);
    if(!scd30. selfCalibrationEnabled(true)) noReturn(0xe0f2);
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
  SerialSW.write(hexdata,9);
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

static int myDummyData=420;

short initSensor() {
  randomSeed(0xc0cac07a);
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

static boolean led = false;

inline void toggleLED(void) {
  digitalWrite(LED_BUILTIN, led=!led);
}

void noReturn(short errorCode) {
  for(;;delay(SHORT_DELAY)) {
    toggleLED();
    if(led) display.showNumberHexEx(errorCode, 0, true);
    else display.clear();
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, INPUT);
  
  Serial.begin(115200);
  delay(SHORT_DELAY);

  display.setBrightness(0x07);
  display.showNumberDec(8888);

  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F("Arduino ESP8266 Ticker test CO2"));
  Serial.println(F(__DATE__  " " __TIME__));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println(F("(C) 2021, IER-UNAM"));
  Serial.println(F("(C) 2021, UNAM"));
  Serial.println(F("*****************************"));
  Serial.println();

  Serial.println(F("Sensor: " SENSOR_NAME));

  delay(LONG_DELAY);

  Serial.println(F("Init..."));
  while(1) {
    toggleLED();
    short st=initSensor();
    delay(SHORT_DELAY);
    if(st==0) break;
    delay(DELAY-SHORT_DELAY);
  }
  
  WiFi.mode(WIFI_STA);
  chkwifi(true);

  String mDNSname = WiFi.macAddress().substring(9);
  mDNSname.toLowerCase();
  mDNSname.replace(":","");
  mDNSname = F("monitorCO2-") + mDNSname;

  Serial.print("mDNS:");
  Serial.println(mDNSname);
  MDNS.begin(mDNSname);

  /*
   * Esta sección está incompleta
   */
  Serial.println(F("WebServer..."));
  server.on(F("/"), handleRoot);
  server.on(F("/commit/"), HTTP_POST, handleCommit);
  server.on(F("/frc/"), handleFRC);
  server.on(F("/asc/"), handleASC);
  
  server.begin();
  /*
   * Termina la sección incompleta
   */

  Serial.println("Ticker...");
  tickerSensor.attach_ms(MEASUREMENT_INTERVAL*DELAY, tckrRead);
  tickerPost.attach(POST_INTERVAL, tckrPost);

  Serial.println("leaving setup()");
}

volatile short co2ppm = -1;
volatile boolean isTime2read = true;
volatile boolean isTime2post = true;

void loop(void) {
  if(isTime2read) { 
    doReadSensor();
    isTime2read=false;
  }
  if(isTime2post) {
    doPost();
    isTime2post=false;
  }
  server.handleClient();
}

void tckrPost() { isTime2post = true; }
void tckrRead() { isTime2read = true; }

void doReadSensor(void) {
  toggleLED();

  co2ppm = readSensor();  

  Serial.print(F("CO2ppm="));
  Serial.println(co2ppm);
  display.showNumberDec(co2ppm);
}

void doPost(void) {
  toggleLED();
  
  if(WiFi.status()!=WL_CONNECTED) chkwifi(false);

  MDNS.update();
  
  WiFiClient wifiClient;
  HTTPClient httpClient;

  String payload = "{";
  payload += "\"" SENSOR_NAME "\":";
  payload += co2ppm;
  payload += "}";
  Serial.print(F("POST: "));
  Serial.println(payload);
#ifdef DEBUG
  Serial.println(PSTR(SECRET_URL));
#endif
  httpClient.begin(wifiClient, PSTR(SECRET_URL));
  httpClient.addHeader(F("Content-Type"), F("application/json"));

  int statusCode = httpClient.POST(payload);
  
  Serial.print(F("ST="));
  Serial.print(statusCode);
  if(statusCode<0) {
    Serial.print(":");
    Serial.print(httpClient.errorToString(statusCode));
  }
  Serial.println();
}

void chkwifi(boolean retry) {
  int st;

  display.showNumberHexEx(0xe000,0,true);

  Serial.print(F("WiFi SSID: "));
  Serial.println(ssid);

  WiFi.begin(ssid,pass);

  while((st = WiFi.status()) != WL_CONNECTED) {
    Serial.print("WiFi.status=");
    Serial.println(wl_status_to_string(st));
    if(!retry) break;
    for(byte i=1; i!=0; i<<=1)
      for(short d=0; d<DELAY; d+=SHORT_DELAY) {
        toggleLED(); 
        delay(SHORT_DELAY);
      }
  }
  if(st == WL_CONNECTED) {
    Serial.print("IP address=");
    Serial.println(WiFi.localIP());
  }
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
