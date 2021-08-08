#define SENSOR_NAME "scd30"

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"
#include <TM1637Display.h>
#include <Adafruit_SCD30.h>

#define WAIT_SERIAL

#define MEASURE_INTERVAL 2
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

Ticker tickerSensor;
Ticker tickerPost;

Adafruit_SCD30 scd30;

TM1637Display display(CLK, DIO);

static boolean led = false;

inline void toggleLED(void) {
  digitalWrite(LED_BUILTIN, led=!led);
}

void noReturn(void) {
  for(;;delay(SHORT_DELAY))toggleLED();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  toggleLED();
  // put your setup code here, to run once:
  Serial.begin(115200);
#ifdef WAIT_SERIAL
  while(!Serial) {
    toggleLED(); delay(SHORT_DELAY);
  }
#endif
  Serial.println();
  Serial.println(F("*****************************"));
  Serial.println(F("Arduino ESP8266 Ticker test"));
  Serial.println(F(__DATE__  " " __TIME__));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println(F("*****************************"));
  Serial.println();

  Serial.println(F("scd30.begin()"));
  if(!scd30.begin()) noReturn();

  Serial.printf(PSTR("Interval=%d\n"), MEASURE_INTERVAL);
  if(scd30.getMeasurementInterval()!=MEASURE_INTERVAL)
    if(!scd30.setMeasurementInterval(MEASURE_INTERVAL)) noReturn();

  tickerSensor.attach(2, tckrReadSensor);
  tickerPost.attach(60, tckrPost);

  Serial.println("leaving setup()");
}

void loop() {}

typedef struct {
  short co2ppm, temp, rh;
} data_t;

volatile data_t data;

void tckrReadSensor(void) {
  toggleLED();

  if (!scd30.read()) { Serial.println("Error reading sensor data"); return; }
  data.co2ppm = scd30.CO2;
  data.temp = scd30.temperature;
  data.rh = scd30.relative_humidity;

  Serial.print(F("CO2ppm="));
  Serial.println(data.co2ppm);
  display.showNumberDec(data.co2ppm);
}

void tckrPost(void) {
  Serial.println(F("tickerSensor.detach()"));
  tickerSensor.detach();
  
  String payload = "{";
  payload += "'" SENSOR_NAME "':";
  payload += data.co2ppm;
  payload += "}";

  if(WiFi.status()!=WL_CONNECTED) chkwifi();
  
  Serial.print(F("POST: "));
  Serial.println(payload);

  yield();
  httpClient.post(SECRET_TOPIC, F("application/json"), payload);
  yield();
  
  int statusCode = httpClient.responseStatusCode();  
  String response = httpClient.responseBody();

  Serial.print("ST=");
  Serial.println(statusCode);

  if(statusCode!=200) chkwifi();

  Serial.println(F("tickerSensor.attach()"));
  tickerSensor.attach(MEASURE_INTERVAL, tckrReadSensor);
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
  scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/false);
  
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

int16_t readSensor(void) {
  static byte b = 0;
  return ++b;
}
