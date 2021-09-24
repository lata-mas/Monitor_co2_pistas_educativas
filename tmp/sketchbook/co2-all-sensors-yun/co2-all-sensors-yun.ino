/*
  CO2-all-sensors

    Requires Arduino Yún 
    Requires Linux script to send data to MQTT broker

  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>

  Quick&Dirty copy-paste samples for CO2 sensors:
  
  * sen0219 analog interface
  * mh-z14a uart interface
  * t3022 i2c
  * senseair s8

*/

#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include <Bridge.h>
#include <SoftwareSerial.h>
#include <TM1637Display.h>

#include "cma.h"

/*
#define DEBUG
*/

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 8129

#define CLK 6
#define DIO 7

TM1637Display display(CLK, DIO);

static const byte SEG_i2c[] = { SEG_C, SEG_A | SEG_B | SEG_G | SEG_E | SEG_D, SEG_G | SEG_E | SEG_D, 0 };
static const byte SEG_s8lp[] = { 0b01101101, 0b01111111, 0b00111000, 0b01110011 };

/*********************** 
 * WDT Interrupt Driver 
 ***********************/
 
volatile short __wdt_counter = 0;

ISR(WDT_vect) {
  ++__wdt_counter;
}

/****************************************
 * Sensor T3022 0-5K CO2 ppm            
 *   i2c addr = 0x15
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#include <Wire.h>

#define I2C_ADDR (byte)0x15

static const byte CMD_FWREV[]  = {0x04, 0x13, 0x89, 0x00, 0x01};
static const byte CMD_STATUS[] = {0x04, 0x13, 0x8A, 0x00, 0x01};
static const byte CMD_CO2PPM[] = {0x04, 0x13, 0x8B, 0x00, 0x01};

#define T3022_DELAY 100

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
  delay(T3022_DELAY); // T67xx CO2 sensor module Application note
  Wire.requestFrom(I2C_ADDR, sizeof(data));
  receiveData(&data, sizeof(data));
  if( (data.fc==4) && (data.bc==2)) {
    retval.hi=data.hi; retval.lo=data.lo;
    return retval.w;
  } else return -1;
}

/****************************************
 * Sensor SEN0220 0-50K CO2 ppm            
 *   Serial1 requires Arduino Leonardo
 *   or similar board
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#define RX_SW 8
#define TX_SW 9

SoftwareSerial SerialSW(RX_SW,TX_SW);

static unsigned char hexdata[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 

typedef struct {
  byte ff, cmd, hi, lo, fill[4], crc;
} data_sen0220_t;

int sen0220(void) {
  register union {
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

/****************************************
 * Sensor SEN0219 0-5K CO2 ppm            
 *   Requieres AREF=3.3V
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#define PIN_SEN0219 A0

#define SAMPLES 15
CMA sen0219cma(SAMPLES);

int sen0219(void) {
  int sensorValue = analogRead(PIN_SEN0219);
  float voltage = sensorValue*(3300/1024.0);
  if(voltage<400) return -1;
  else {
    int voltage_diference=voltage-400;
    return int(voltage_diference*50.0/16.0);
  }
}

/****************************************
 * SenseAir S8            
 *   UART interface
 *   (CC) 2021 <hdcg@ier.unam.mx>
 ****************************************/
#define RX_S8 10
#define TX_S8 11

SoftwareSerial SerialS8(RX_S8,TX_S8);

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

int s8co2read(void) {
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

/*********************
 * Main program here!
 *********************/
boolean led = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, led=!led);

  analogReference(EXTERNAL);

  display.setBrightness(0x07);
  display.showNumberDec(8888);
  delay(LONG_DELAY);
  
  //Initialize serial and wait for port to open:
  Serial.begin(38400);
#ifdef DEBUG
  while (!Serial) {
    digitalWrite(LED_BUILTIN, led=!led);
    delay(SHORT_DELAY);
  }
#endif
  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F(" CO2 all sensors Yún version "));
  Serial.println(F(" * sen0219\n * mh-z14a\n * t3022\n * s8lp"));
  Serial.println(F(" (C) 2021 hdcg@ier.unam.mx"));
  Serial.println(F("*****************************"));
  Serial.println();

  Serial.print(F("Initializing SerialSW..."));
  SerialSW.begin(9600);
  Serial.println(F("\nDone!"));

  display.setSegments(SEG_i2c);
  delay(SHORT_DELAY);
  
  Wire.begin();
  
  Serial.println(F("Detect[t3022]..."));
  Serial.print(F("FWREV=0x"));
  int fwrev=t3022(CMD_FWREV);
  display.showNumberHexEx(fwrev,0,true);
  delay(SHORT_DELAY);
  Serial.println(fwrev, HEX);

  Serial.println(F("Init[t3022]..."));
  while(1) {
    Serial.print(F("ST=0x"));
    digitalWrite(LED_BUILTIN, led=!led);
    word st=t3022(CMD_STATUS);
    display.showNumberHexEx(st,0,true);
    Serial.println(st, HEX);
    delay(SHORT_DELAY);
    if(st==0) break;
    delay(DELAY-SHORT_DELAY);
  }

  Serial.print(F("Initializing SerialS8..."));
  SerialS8.begin(9600);
  Serial.println(F("\nDone!"));

  display.setSegments(SEG_s8lp);
  delay(SHORT_DELAY);
  Serial.println(F("Init[s8lp]..."));
  while(1) {
    Serial.print(F("ST=0x"));
    digitalWrite(LED_BUILTIN, led=!led);
    word st=s8status();
    display.showNumberHexEx(st,0,true);
    Serial.println(st, HEX);
    delay(SHORT_DELAY);
    if(st==0) break;
  }

  Serial.print(F("Bridge.begin()..."));
  display.showNumberDec(1234);
  digitalWrite(LED_BUILTIN, led=!led);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, led=!led);
  Serial.println(F("\nDone!"));

  Serial.print(F("Running shell script..."));
  Process p;
  digitalWrite(LED_BUILTIN, led=!led);
  display.showNumberDec(2345);
  p.runShellCommand("/bin/sh /root/arduino2thingsboard.sh &> /dev/null &");
  digitalWrite(LED_BUILTIN, led=!led);
  Serial.println(F("\nDone!"));

  display.showNumberDec(3456);
  Serial.print(F("Setting WDT Interrupt"));
// WDT Timer Setup, 1024 ms
  MCUSR &= ~(1<<WDRF);
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = 1<<WDP1 | 1<<WDP2; // 1s
  WDTCSR |= _BV(WDIE);
  Serial.println(F("\nDone!"));
  display.clear();
}

byte interval = 0;

void loop() {
  if(__wdt_counter==0) return;
  --__wdt_counter; 

// Heartbeat
  unsigned long heartbeat=millis();
  unsigned long uptime=heartbeat/1000;

  display.showNumberDec(uptime%10000,uptime>=10000);
  digitalWrite(LED_BUILTIN, led=!led);

  Serial.print(String(heartbeat) + " " + uptime);
  
  Bridge.put(F("heartbeat"),String(heartbeat));
  Bridge.put(F("uptime"),String(uptime));

  int co2sen0219, co2sen0219avg, co2mh_z14a, co2t3022, co2s8lp;
  
  switch(interval++) {
    case 0:
      co2sen0219 = sen0219();
      Bridge.put(F("sen0219"),String(co2sen0219));  
      sen0219cma.addData(co2sen0219);
      co2sen0219avg = sen0219cma.avg();
      Bridge.put(F("sen0219avg"),String(co2sen0219avg));
      break;
    case 1:
      co2mh_z14a = sen0220();
      Bridge.put(F("mh-z14a"),String(co2mh_z14a));  
      break;
    case 2:
      co2t3022 = t3022(CMD_CO2PPM);
      Bridge.put(F("t3022"),String(co2t3022));  
      break;
    case 3:
      co2s8lp = s8co2read();
      Bridge.put(F("s8lp"),String(co2s8lp));  
      interval = 0;
      break;
  }

  Serial.println(String(" ") + co2sen0219 + " " + co2sen0219avg + " " + co2mh_z14a + " " + co2t3022 + " " + co2s8lp);
}
