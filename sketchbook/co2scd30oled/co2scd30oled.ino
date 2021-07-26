//#define WAIT_SERIAL
#define DEBUG
#define EPD
//#define OLED

//#include <Adafruit_SleepyDog.h>
#include <ArduinoLowPower.h>

#if defined _ARDUINO_LOW_POWER_H_
#define SLEEP_METHOD "LOWPOWER"
#elif defined ADAFRUIT_SLEEPYDOG_H
#define SLEEP_METHOD "SLEEPYDOG"
#else
#define SLEEP_METHOD "DELAY"
#endif

#include <Adafruit_SCD30.h>

Adafruit_SCD30 scd30;

#define RDY_PIN 7
#define MSR_INTERVAL 60

#ifdef OLED
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold24pt7b.h>

#define FG_COLOR WHITE
#define BG_COLOR BLACK

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
#endif

#ifdef EPD
#include "Adafruit_ThinkInk.h"
#include <Fonts/FreeSansBold24pt7b.h>

#define EPD_CS      5
#define EPD_DC      4
#define SRAM_CS     3
#define EPD_RESET   -1 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY    -1 // can set to -1 to not use a pin (will wait a fixed delay)

#define FG_COLOR EPD_BLACK
#define BG_COLOR EPD_WHITE

// 1.54" Monochrome displays with 200x200 pixels and SSD1681 chipset
ThinkInk_154_Mono_D67 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

#endif

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ESP8266_ESP12)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
  #include <WiFiClient.h>
#endif

#include <ArduinoHttpClient.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// To connect with SSL/TLS:
//   Change WiFiClient to WiFiSSLClient.

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SECRET_SERVER, SECRET_PORT);

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 4096

volatile unsigned long __wakeUpISR = 0;

void myWakeUpISR(void) {
  ++__wakeUpISR;
}

boolean led = false;

inline void toggleLED(void) {
  digitalWrite(LED_BUILTIN, led=!led);
}

void noReturn(void) {
  while(1) {
    toggleLED();
    delay(SHORT_DELAY);
  }
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  toggleLED();
  pinMode(RDY_PIN, INPUT_PULLDOWN);

#ifdef DEBUG
  Serial.begin(38400);
#ifdef WAIT_SERIAL
  while(!Serial) {
    toggleLED();
    delay(SHORT_DELAY);
  }
#endif
  Serial.println("\n" __FILE__ "\n" __DATE__ " " __TIME__);
  Serial.println(SLEEP_METHOD);
  
#endif

#ifdef EPD
  Serial.println("EPD...");
  display.begin(THINKINK_MONO);
  //IER-SPLASH
  display.clearBuffer();
  display.display();
  delay(LONG_DELAY);
  display.setTextSize(2);
#endif

#ifdef OLED
  Serial.println("OLED...");
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { // Address 0x3C for 128x32
      noReturn();
  }
  display.display();
  delay(LONG_DELAY);
  display.setTextSize(1);
#endif

  display.setTextColor(FG_COLOR);
#ifdef EPD
  display.clearBuffer();
#else
  display.clearDisplay();
#endif

  display.println("IER-UNAM CO2 project");
  display.println(" Sensirion scd30");
  display.println();
#ifdef OLED
  display.display();
  delay(DELAY);
#endif

  Serial.println("Detect...");
  display.print("Detect... ");
#ifdef OLED
  display.display();
#endif
  if (!scd30.begin()) {
    display.println("Failed!");
    display.display();
    noReturn();
  }
  display.println("Ok!");
#ifdef OLED
  display.display();
  delay(SHORT_DELAY);
#endif
  
  Serial.println("Init...");
  display.print("Init... ");
#ifdef OLED
  display.display();
#endif
  if (!scd30.setMeasurementInterval(MSR_INTERVAL)){
    display.println("Failed!");
    display.display();
    noReturn();
  }
  display.println("Ok!");
#ifdef OLED
  display.display();
  delay(SHORT_DELAY);
#endif
  
  display.print("Interval="); 
  display.print(scd30.getMeasurementInterval()); 
  display.println("s");
#ifdef OLED
  display.display();
  delay(SHORT_DELAY);
#endif

  display.println("RDY_ISR()");
  display.print(SLEEP_METHOD);
#ifdef OLED
  display.display();
#endif

  Serial.println("RDY_ISR()...");
#ifdef _ARDUINO_LOW_POWER_H_
  LowPower.attachInterruptWakeup(RDY_PIN, myWakeUpISR, RISING);
#else
  int RDY_INT=digitalPinToInterrupt(RDY_PIN);
  attachInterrupt(RDY_INT, myWakeUpISR, RISING);
#endif

  Serial.println("WiFi...");

  chkwifi();

  Serial.println("leaving setup()");
#ifdef _ARDUINO_LOW_POWER_H_
  digitalWrite(LED_BUILTIN, LOW);
#else
  toggleLED();
#endif
}

void loop() {
#ifndef _ARDUINO_LOW_POWER_H_
  toggleLED();
#endif

  if (scd30.dataReady()) {
#ifdef _ARDUINO_LOW_POWER_H_
    digitalWrite(LED_BUILTIN, HIGH);
#endif
#ifdef EPD
    display.clearBuffer();
#else
    display.clearDisplay();
#endif
    display.setCursor(0,56);
    display.print(__wakeUpISR, HEX);

    if (!scd30.read()){
      display.setCursor(0,8);
      display.println("Read Error!");
      display.display();
      return;
    }

    display.setFont(&FreeSansBold24pt7b);
#ifdef EPD
    display.setCursor(0,128);
#else
    display.setCursor(0,56);
#endif
    String value;
    for(value = String(scd30.CO2, 0); value.length()<4; value=" " + value);
    display.print(value);
    display.setFont();

    display.setCursor(108,24);
    display.print("CO2");
    display.setCursor(108, 32);
    display.print("ppm");

    unsigned long heartbeat = millis();
    unsigned long uptime = heartbeat/1000;

    String payload = "{";
    payload += "'heartbeat':";
    payload += heartbeat;
    payload += ",'uptime':";
    payload += uptime;
    payload += ",'wakeUpISR':";
    payload += __wakeUpISR;
    payload += ",'scd30':";
    payload += String(scd30.CO2, 0);
    payload += ",'T':";
    payload += String(scd30.temperature, 0);
    payload += ",'RH':";
    payload += String(scd30.relative_humidity, 0);
    payload += "}";

    display.setCursor(0,0);
    display.println(ssid);
    display.print("POST:");
#ifdef OLED
    display.display();
#endif

    Serial.print("POST: ");
    Serial.println(payload);

    httpClient.post(SECRET_TOPIC, "application/json", payload);

    int statusCode = httpClient.responseStatusCode();  
    String response = httpClient.responseBody();

    Serial.print("ST=");
    Serial.println(statusCode);

    display.print(statusCode);
    display.display();

    if(statusCode!=200) chkwifi();
#ifdef _ARDUINO_LOW_POWER_H_
    digitalWrite(LED_BUILTIN, LOW);
#endif
  } else 
#if defined _ARDUINO_LOW_POWER_H_
  LowPower.sleep();
#elif defined ADAFRUIT_SLEEPYDOG_H
  Watchdog.sleep(DELAY);
#else
  delay(DELAY);
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
}

void chkwifi(void) {
  int st;
  WiFi.disconnect();

  display.fillRect(0,0,127,15, BG_COLOR);
  display.setCursor(0,0);
  display.println(ssid);
  display.print("CONNECTING WIFI...");
  display.display();

  for(byte k=1; st=WiFi.begin(ssid, pass) != WL_CONNECTED; (k<<=1)==0?k=1:k) {
    display.fillRect(0,8,127,15, BG_COLOR);
    display.setCursor(0,8);
    display.print(wl_status_to_string(st));
    display.print(" 0x");
    display.print(k, HEX);
    display.display();
    for(unsigned i=0; i<=k*LONG_DELAY; i+=SHORT_DELAY) {
      toggleLED();
      delay(SHORT_DELAY);
    }
  }
  display.fillRect(0,8,127,15, BG_COLOR);
  display.setCursor(0,8);
  display.print("OK!");
  display.display();
}
