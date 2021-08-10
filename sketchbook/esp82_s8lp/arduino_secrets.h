#ifndef _ARDUINO_SECRETS_H_
#define _ARDUINO_SECRETS_H_

#define SECRET_SSID "TOTALPLAY_9A0650" //IER //TOTALPLAY_9A0650
#define SECRET_PASS "W1D41WSC17g" //acadier2014 //W1D41WSC17g

#ifdef _ARDUINO_MQTT_CLIENT_H_
#define SECRET_BROKER "iot.ier.unam.mx"
#define SECRET_PORT 1883
#define SECRET_DEVID "d147d250-d24a-11eb-897a-cd1c177f68f0"
#define SECRET_TOKEN "nCsfwwiNH8JpXLmI2yuY"
#define SECRET_TOPIC "v1/devices/me/telemetry"
#endif

#ifdef ArduinoHttpClient_h
#define SECRET_SERVER "iot.ier.unam.mx"
#define SECRET_PORT 8080
#define SECRET_TOPIC "/api/v1/nCsfwwiNH8JpXLmI2yuY/telemetry"
#endif

#endif
