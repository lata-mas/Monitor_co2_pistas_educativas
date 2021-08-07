#include <ESP8266WiFi.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

#define DUMMY
#define SENSOR_NAME "dummy"

#define WAIT_SERIAL

#define MEASURE_INTERVAL 2
#define POST_INTERVAL    60
#define DISPLAY_INTERVAL 2

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 8192

char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS; 

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SECRET_SERVER, SECRET_PORT);

static boolean led = false;

inline void toggleLED(void) {
  digitalWrite(LED_BUILTIN, led=!led);
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
  Serial.println(F("Arduino ESP8266 Test Skectch"));
  Serial.println(F(__DATE__  " " __TIME__));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println(F("*****************************"));
  Serial.println();

  chkwifi();
}

int8_t measure_interval = 0;
int8_t post_interval = 0;
int8_t display_interval = 0;

void loop() {
  int16_t co2ppm;
  
  toggleLED();

  if(measure_interval--==0) {
    measure_interval=MEASURE_INTERVAL;
    co2ppm = readSensor();
    Serial.printf(PSTR("CO2=%d\n"), co2ppm);
  }

  if(post_interval--==0) {
    post_interval=POST_INTERVAL;

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

  if(display_interval--==0) {
    display_interval=DISPLAY_INTERVAL;

    Serial.println("DISPLAY NOT IMPLEMENTED");
  }
  delay(DELAY);
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
