#define SENSOR_NAME "scd30"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoHttpClient.h>
#include <TM1637Display.h>
#include <Adafruit_SCD30.h>
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

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SECRET_SERVER, SECRET_PORT);

Adafruit_SCD30 scd30;

TM1637Display display(CLK, DIO);

static boolean led = false;

inline void toggleLED(void) {
  digitalWrite(LED_BUILTIN, led=!led);
}

inline void noReturn(void) {
  for(;;delay(SHORT_DELAY))toggleLED();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  toggleLED();

  Serial.begin(115200);

  display.setBrightness(0x07);
  display.showNumberDec(8888);
  delay(LONG_DELAY);

  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F("Arduino ESP8266 Test Skectch"));
  Serial.println(F(__DATE__  " " __TIME__));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println(F("*****************************"));
  Serial.println();
  delay(SHORT_DELAY);
  
  Serial.println(F("scd30.begin()"));
  if(!scd30.begin()) noReturn();
  delay(SHORT_DELAY);
  
  Serial.print(F("Interval="));
  Serial.println(MEASUREMENT_INTERVAL);
  if(scd30.getMeasurementInterval()!=MEASUREMENT_INTERVAL) {
    delay(SHORT_DELAY);
    if(!scd30.setMeasurementInterval(MEASUREMENT_INTERVAL)) noReturn();
  }
  delay(SHORT_DELAY);

  WiFi.mode(WIFI_STA);
  chkwifi();

  Serial.println(F("leaving setup()"));
}

int8_t post_interval = 0;

void loop() {
  static int16_t co2ppm;

  unsigned long entryTick=millis();

  toggleLED();

  if (scd30.dataReady()) {
    if (!scd30.read()) Serial.println("Error reading sensor data");
    else {
      co2ppm = scd30.CO2;

      Serial.print(F("CO2ppm="));
      Serial.println(co2ppm);
      display.showNumberDec(co2ppm);
    }
  }

  if(post_interval--==0) {
    post_interval=POST_INTERVAL-1;

    String payload = "{";
    payload += "'" SENSOR_NAME "':";
    payload += co2ppm;
    payload += "}";

    if(WiFi.status()!=WL_CONNECTED) chkwifi();
    
    Serial.print("POST: ");
    Serial.println(payload);

    httpClient.post(SECRET_TOPIC, F("application/json"), payload);

    int statusCode = httpClient.responseStatusCode();  
    String response = httpClient.responseBody();

    Serial.print("ST=");
    Serial.println(statusCode);

    if(statusCode!=200) chkwifi();
  }

  unsigned long exitTick=millis();
  unsigned long nextTick=exitTick-entryTick;
  if(nextTick<DELAY) delay(DELAY-nextTick);
}

void chkwifi(void) {
  int st;
//  WiFi.disconnect();
//  delay(DELAY);

  Serial.println(F("Attempting to connect to WPA SSID: "));
  Serial.println(ssid);

  st = WiFi.begin(ssid, pass);
  if(st == WL_CONNECTED) {
    Serial.println(F("You're connected to the network"));
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("ST=");
    Serial.println(wl_status_to_string(st));
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
}
