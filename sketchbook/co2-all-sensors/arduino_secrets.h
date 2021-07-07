#ifndef _ARDUINO_SECRETS_H_
#define _ARDUINO_SECRETS_H_

#define SECRET_SSID "IER"
#define SECRET_PASS "acadier2014"

#ifdef _ARDUINO_MQTT_CLIENT_H_
#define SECRET_BROKER "iot.ier.unam.mx"
#define SECRET_PORT 1883
#define SECRET_DEVID "a12168c0-db50-11eb-897a-cd1c177f68f0"
#define SECRET_TOKEN "moWNVYpt9hNWaZSKjsun"
#define SECRET_TOPIC "v1/devices/me/telemetry"
#endif

#ifdef ArduinoHttpClient_h
#define SECRET_SERVER "iot.ier.unam.mx"
#define SECRET_PORT 8080
#define SECRET_TOPIC "/api/v1/moWNVYpt9hNWaZSKjsun/telemetry"
#endif

#endif
