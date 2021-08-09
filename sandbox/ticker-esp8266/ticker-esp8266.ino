#define DEBUG
#define DUMMY

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>
#include <TM1637Display.h>
#include "arduino_secrets.h"

#define MEASUREMENT_INTERVAL 2
#define POST_INTERVAL    60

#define CLK 14
#define DIO 12

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 8192

char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS; 

Ticker tickerSensor;
Ticker tickerPost;

TM1637Display display(CLK, DIO);

#ifdef SCD30
/****************************************
 * Sensor SCD30 0-10K CO2 ppm            
 *   i2c addr = 0x61
 *   Thanks Lady Ada!
 ****************************************/
#include <Adafruit_SCD30.h>

Adafruit_SCD30 scd30;

int initSensor() {
  Serial.println(F("scd30.begin()"));
  if(!scd30.begin()) noReturn();

  Serial.printf(PSTR("Interval=%d\n"), MEASUREMENT_INTERVAL);
  if(scd30.getMeasurementInterval()!=MEASUREMENT_INTERVAL)
    if(!scd30.setMeasurementInterval(MEASUREMENT_INTERVAL)) noReturn();

  return 0;  
}

int readSensor() {
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

unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 

typedef struct {
  byte ff, cmd, hi, lo, fill[4], crc;
} data_sen0220_t;

int sen0220(void) {
  union {
    struct { byte lo, hi; };
    word w;
  } retval;
  data_sen0220_t data;
  Serial1.write(hexdata,9);
  delay(500);
  if (Serial1.available()>0) { 
    Serial1.readBytes((char*)&data, 9);
    if((data.ff != 0xff) && (data.cmd != 0x86)) return -1;
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
  else return -1;    
}

inline int initSensor() {
  if(sen0220()==-1) return -1;
  else return 0;
}

inline int readSensor() {
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

int s8status(void) {
  retval_t retval;
  s8data_t data;
  
  Serial1.write((char*)S8_CMD_STATUS, sizeof(S8_CMD_STATUS));
  delay(S8_WAIT_DELAY);
  if (Serial1.available()>0)
    Serial1.readBytes((char*)&data,sizeof(data));
  else return -1;
  
  if((data.addr != 0xfe) && (data.function_code != 0x04)) return -1;
  else {
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
}

int s8co2read(void) {
  retval_t retval;
  s8data_t data;
  
  Serial1.write((char*)S8_CMD_CO2READ, sizeof(S8_CMD_CO2READ));
  delay(S8_WAIT_DELAY);
  if (Serial1.available()>0)
    Serial1.readBytes((char*)&data,sizeof(data));
  else return -1;

  if((data.addr != 0xfe) && (data.function_code != 0x04)) return -1;
  else {
    retval.hi=data.hi;
    retval.lo=data.lo;
    return retval.w;
  }
}

inline int initSensor() {
  return s8status();
}

inline int readSensor() {
  return s8co2read();
}

#define SENSOR_NAME "s8lp"
#endif 

#ifdef DUMMY
/****************************************
 * Dummy Sensor
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
static int myDummyData=420;

int initSensor() {
  randomSeed(0xc0cac07a);
  if (millis()&1!=0) return 0;
  else return -1;
}

int readSensor() {
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

void noReturn(void) {
  for(;;delay(SHORT_DELAY))toggleLED();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  display.setBrightness(0x07);
  display.showNumberDec(8888);
  delay(LONG_DELAY);

  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F("Arduino ESP8266 Ticker test"));
  Serial.println(F(__DATE__  " " __TIME__));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println(F("*****************************"));
  Serial.println();

  Serial.println(F("Sensor: " SENSOR_NAME));

  display.showNumberHexEx(0xE001,0,true);
  Serial.println(F("Init[" SENSOR_NAME "]..."));
  while(1) {
    Serial.print(F("ST=0x"));
    toggleLED();
    short st=initSensor();
    display.showNumberHexEx(st,0,true);
    Serial.println(st, HEX);
    delay(SHORT_DELAY);
    if(st==0) break;
    delay(DELAY-SHORT_DELAY);
  }
  
  WiFi.mode(WIFI_STA);
  chkwifi(true);
  
  tickerSensor.attach(MEASUREMENT_INTERVAL, tckrReadSensor);
  tickerPost.attach(POST_INTERVAL, tckrPost);

  Serial.println("leaving setup()");
}

void loop() {}

volatile short co2ppm;

void tckrReadSensor(void) {
  toggleLED();

  co2ppm = readSensor();  
  
  Serial.print(F("CO2ppm="));
  Serial.println(co2ppm);
  display.showNumberDec(co2ppm);
}

void tckrPost(void) {
  if(WiFi.status()!=WL_CONNECTED) chkwifi(false);
  
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


/*
void scanwifi(void) {
  String ssid;
  int32_t rssi;
  uint8_t encryptionType;
  uint8_t* bssid;
  int32_t channel;
  bool hidden;

  int scanResult;
  int st;

  wifi_network_t* p;

  WiFi.disconnect();
  delay(DELAY);

  Serial.println(F("WiFi scan..."));
  scanResult = WiFi.scanNetworks(false,false);
  //                scanNetworks(async,hidden);
  
  if (scanResult > 0) {
    for(p=SECRET_NETWORKS; p; ++p) {
      Serial.println(p->ssid);
      for (int8_t i = 0; i < scanResult; i++) {
        WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);
        if(ssid == p->ssid) {
          Serial.println(F("Connecting..."));
          st = WiFi.begin(p->ssid, p->passwd);
          if(st == WL_CONNECTED) {
            Serial.println(F("You're connected to the network"));
          } else {
            Serial.print(F("Error status="));
            Serial.println(wl_status_to_string(st));
          }
          return;
        }
      }
    }
  }
}
*/
