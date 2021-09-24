#include <ArduinoHttpClient.h>
#include <ESP8266WiFi.h>
#include "TM1637Display.h"
#include "SoftwareSerial.h"
SoftwareSerial mySerial(13, 15); // RX, TX


#define SHORT_DELAY 128
#define DELAY 1024
#define LONG_DELAY 4096

TM1637Display displayseg(14,12);


//Definiendo pines para leds colores
int pinRojo = 16;
int pinVerde = 0;
int pinAzul = 2;

int Buzzer = D1;

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SECRET_SERVER, SECRET_PORT);

int status = WL_IDLE_STATUS;

int i = 0;
int e = 0;



#define S8LP

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
  
  mySerial.write((char*)S8_CMD_STATUS, sizeof(S8_CMD_STATUS));
  delay(S8_WAIT_DELAY);
  if (mySerial.available()>0)
    mySerial.readBytes((char*)&data,sizeof(data));
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
  
  mySerial.write((char*)S8_CMD_CO2READ, sizeof(S8_CMD_CO2READ));
  delay(S8_WAIT_DELAY);
  if (mySerial.available()>0)
    mySerial.readBytes((char*)&data,sizeof(data));
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

void setup() {

  displayseg.setBrightness(7,true);
  displayseg.showNumberDec(8888,false,4,0);
  displayseg.clear();

  pinMode (Buzzer, OUTPUT);
  digitalWrite (Buzzer, LOW);  //turn buzzer off

  pinMode(pinRojo, OUTPUT);
  pinMode(pinVerde, OUTPUT);
  pinMode(pinAzul, OUTPUT);

  Serial.begin(9600);
  while (!Serial) {

  }
  mySerial.begin(9600);

  //Initialize serial and wait for port to open:
  Serial.println(F("\n***************************"));
  Serial.println(F(" CO2 all sensors template"));
  Serial.println(F(" * sen0219\n * mh-z14a\n * sen0220\n * t3022\n * s8lp"));
  Serial.println(F(" (C) 2021 hdcg@ier.unam.mx"));
  Serial.println(F("***************************\n"));

  Serial.print(F("Sensor: " SENSOR_NAME));
  Serial.println("");

  Serial.println(F("Init[" SENSOR_NAME "]..."));
  while(1) {
    Serial.print(F("ST=0x"));
    //toggleLED();
    word st=initSensor();
   // display.showNumberHexEx(st,0,true);
    Serial.println(st, HEX);
    delay(SHORT_DELAY);
    if(st==0) break;
    delay(DELAY-SHORT_DELAY);
  }
   
   InitWiFi();
   displayseg.clear();
  
}

void loop () {
  
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
  }

  else {
    
    
    int co2ppm = readSensor(); 
    displayseg.showNumberDec(co2ppm,false,4,0);

     buzz1();
     buzz2();

    if (co2ppm <= 600) {
      definirColor(0, 1023, 0,true);  // verde
    }
    
    if ((co2ppm >= 601) && (co2ppm <= 750)) {
      definirColor(1023, 1023, 0,true);  // amarillo
    }

    if (co2ppm >= 751)  {
      definirColor(1023, 0, 0,true);  // rojo
    }


    String payload = "{";
    payload += "'s8lp':";
    payload += co2ppm;
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

void InitWiFi()
{
   Serial.println("Connecting to AP ...");
   Serial.println(ssid);
  // attempt to connect to WiFi network

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    displayseg.showNumberDec(1234,false,4,0);
    if  ( e < 5) {
      delay(3000);
      Serial.print(".....................");
      e++;
      if (e == 5) {
        reconnect();
      }
     }
    }
  Serial.println("Connected to AP");
  
}

void reconnect() {
  
 // Loop until we're reconnected
  status = WiFi.status();
  while (WiFi.status() != WL_CONNECTED) {
     if  ( i < 10) {
      Serial.println("Reintentando conexion...");
      WiFi.begin(ssid, pass);
      delay(5000);
      int co2ppm = readSensor(); 

    if (co2ppm <= 600) {
      definirColor(0, 1023, 0,true);  // verde
    }
    
    if ((co2ppm >= 601) && (co2ppm <= 750)) {
      definirColor(1023, 1023, 0,true);  // amarillo
    }

    if (co2ppm >= 751)  {
      definirColor(1023, 0, 0,true);  // rojo
    }
 
      String payload = "{";
      payload += "'s8lp':";
      payload += co2ppm;
      payload += "}";

      Serial.println("POST... " + payload);
      i++;
      if (i == 10) {
        
        Serial.println("Conexion Fallida, reset en 3 seg");
        delay(3000);
        ESP.reset();
      }
     } 
     }
   Serial.println("Connected to AP");
   
}


void definirColor(int rojo, int verde, int azul, boolean anodoComun)
{
  if(anodoComun == true)
  {
    rojo = 1023 - rojo;
    verde = 1023 - verde;
    azul = 1023 - azul;
  }
  analogWrite(pinRojo, rojo);
  analogWrite(pinVerde, verde);
  analogWrite(pinAzul, azul); 
}


void buzz1(){
  
  int co2ppm = readSensor(); 
  if ((co2ppm >= 601) && (co2ppm <= 750)) {
  digitalWrite (Buzzer, HIGH); //turn buzzer on
  delay(100);
  digitalWrite (Buzzer, LOW);  //turn buzzer off
  delay(50);
  digitalWrite (Buzzer, HIGH); //turn buzzer on
  delay(100);
  digitalWrite (Buzzer, LOW);  //turn buzzer off
  
}
}


void buzz2(){
  
  int co2ppm = readSensor(); 
  
  if (co2ppm >= 751)  {
  digitalWrite (Buzzer, HIGH); //turn buzzer on
  delay(100);
  digitalWrite (Buzzer, LOW);  //turn buzzer off
  delay(50);
  digitalWrite (Buzzer, HIGH); //turn buzzer on
  delay(100);
  digitalWrite (Buzzer, LOW);  //turn buzzer off
  delay(50);
  digitalWrite (Buzzer, HIGH); //turn buzzer on
  delay(100);
  digitalWrite (Buzzer, LOW);  //turn buzzer off
  
}
}
