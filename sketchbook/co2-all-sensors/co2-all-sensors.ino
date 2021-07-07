#define DEBUG
/*
  CO2-all-sensors

    Requires Arduino Uno WiFi Rev.2 board

  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>

  Supported sensors:

  * DUMMY
    A dummy sensor for test purposes
    
  * SEN0219
    Analog interface for DFRobot SEN0219. Deprecated.
    
  * SEN0220
    DFRobot SEN0220.

  * MH_Z14a
    UART interface for DFRobot SEN0219 AKA MH-Z14A
    
  * T3022
    TelAire T3022 OEM. i2c (TWI) interface.
  
  * S8LP
    SenseAir S8 Low Power. UART interface.
  
*/
#define DUMMY
#define POST_INTERVAL_MS 60000L

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ESP8266_ESP12)
  #include <ESP8266WiFi.h>
#endif

#include <TM1637Display.h>

//#include <ArduinoMQTTClient.h>
#include <ArduinoHttpClient.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// To connect with SSL/TLS:
//   Change WiFiClient to WiFiSSLClient.

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SECRET_SERVER, SECRET_PORT);

const long interval = POST_INTERVAL_MS;
unsigned long previousMillis = 0;

#define SHORT_DELAY 128
#define DELAY 1024
#define LONG_DELAY 4096

#define CLK 6
#define DIO 7

TM1637Display display(CLK, DIO);

#ifdef T3022
/****************************************
 * Sensor T3022 0-5K CO2 ppm            
 *   i2c addr = 0x15
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#include <Wire.h>

#define I2C_ADDR (byte)0x15

static const byte CMD_FWREV[] = {0x04, 0x13, 0x89, 0x00, 0x01};
static const byte CMD_STATUS[] = {0x04, 0x13, 0x8A, 0x00, 0x01};
static const byte CMD_CO2PPM[] = {0x04, 0x13, 0x8B, 0x00, 0x01};

typedef struct {
  byte fc, bc, hi, lo;
} data_t3022_t;

void sendData(const void* p, size_t n) {
  byte i;
  for(i=0; i<n; ++i)
    Wire.write(((byte*)p)[i]);
}

void receiveData(const void* p, size_t n) {
  byte i;
  for(i=0; i<n; ++i)
    ((byte*)p)[i]=Wire.read();
}

word t3022(const void* p) {
  union {
    struct {
      byte lo,hi;
    };
    word w;
  } retval;
  data_t3022_t data;
  Wire.beginTransmission(I2C_ADDR);
  sendData(p, 5);
  Wire.endTransmission();
  delay(50); // T67xx CO2 sensor module Application note
  Wire.requestFrom(I2C_ADDR, sizeof(data));
  receiveData(&data, sizeof(data));
  if( (data.fc==4) && (data.bc==2)) {
    retval.hi=data.hi; retval.lo=data.lo;
    return retval.w;
  } else return -1;
}

inline int detectSensor() {
   return t3022(CMD_FWREV); 
}

inline int initSensor() {
  return t3022(CMD_STATUS);
}

inline int readSensor() {
  return t3022(CMD_CO2PPM);
}

#define SENSOR_NAME "t3022"
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
  register union {
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

int initSensor() {
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

#ifdef SEN0219
/****************************************
 * Sensor SEN0219 0-5K CO2 ppm            
 *   Requieres AREF=3.3V
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#define PIN_SEN0219 A0

int sen0219(void) {
  int sensorValue = analogRead(PIN_SEN0219);
  float voltage = sensorValue*(3300/1024.0);
  if(voltage<400) return -1;
  else {
    int voltage_diference=voltage-400;
    return int(voltage_diference*50.0/16.0);
  }
}

int initSensor() {
  if(sen0219()==-1) return -1;
  else return 0;
}

inline int readSensor() {
  return sen0219();
}

#define SENSOR_NAME "sen0219"
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

/*********************
 * Main program here!
 *********************/
#ifndef SENSOR_NAME
#error "PLEASE SELECT A SENSOR"
#endif

#ifdef TwoWire_h
static const byte SEG_i2c[] = { SEG_C, SEG_A | SEG_B | SEG_G | SEG_E | SEG_D, SEG_G | SEG_E | SEG_D, 0 };
#endif

boolean led = false;

inline void toggleLED(void) {
  digitalWrite(LED_BUILTIN, led=!led);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  toggleLED();
  
  analogReference(EXTERNAL);

  display.setBrightness(0x07);
  display.showNumberDec(8888);
  delay(LONG_DELAY);
  
  //Initialize serial and wait for port to open:
  Serial.begin(38400);
  Serial.println(F("\n***************************"));
  Serial.println(F(" CO2 all sensors template"));
  Serial.println(F(" * sen0219\n * mh-z14a\n * sen0220\n * t3022\n * s8lp"));
  Serial.println(F(" (C) 2021 hdcg@ier.unam.mx"));
  Serial.println(F("***************************\n"));

  Serial.print(F("Sensor: " SENSOR_NAME));

  Serial1.begin(9600);

#ifdef TwoWire_h
  display.setSegments(SEG_i2c);
  delay(SHORT_DELAY);

  Wire.begin();
#endif

#ifdef T3022
  Serial.println(F("Detect[t3022]..."));
  Serial.print(F("FWREV=0x"));
  int fwrev=t3022(CMD_FWREV);
  display.showNumberHexEx(fwrev,0,true);
  delay(SHORT_DELAY);
  Serial.println(fwrev, HEX);
#endif

  Serial.println(F("Init[" SENSOR_NAME "]..."));
  while(1) {
    Serial.print(F("ST=0x"));
    toggleLED();
    word st=initSensor();
    display.showNumberHexEx(st,0,true);
    Serial.println(st, HEX);
    delay(SHORT_DELAY);
    if(st==0) break;
    delay(DELAY-SHORT_DELAY);
  }

  display.showNumberDec(1234);
  // attempt to connect to Wifi network:
  Serial.print(F("Attempting to connect to WPA SSID: "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    toggleLED();
    Serial.print(F("."));
    delay(DELAY);
  }

  Serial.println(F("You're connected to the network"));

  Serial.print(F("POST interval="));
  Serial.print(interval);
  Serial.println(F("ms\n"));

  display.clear();
}

void loop() {
  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    unsigned long uptime = (previousMillis = currentMillis) / 1000;

    toggleLED();

    // Sensor Data HERE! 
    int co2ppm = readSensor(); 

    // Common code here!
    display.showNumberDec(co2ppm);
    
    String payload = "{";
    payload += "'" SENSOR_NAME "':";
    payload += co2ppm;
    payload += "}";

    Serial.println("POST... " + payload);

    toggleLED();
    httpClient.post(SECRET_TOPIC, "application/json", payload);

    int statusCode = httpClient.responseStatusCode();  
    String response = httpClient.responseBody();
    Serial.print("Status code: ");
    Serial.print(statusCode);
    Serial.print(" ");
    Serial.println(response);
    toggleLED();
  }
}
