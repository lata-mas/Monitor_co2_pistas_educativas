#ifndef ARDUINO_ARCH_ESP8266
#error Requires esp8266
#endif 
#include <Ticker.h>

Ticker tickerButton;
Ticker tickerBuzzer;

#define BUZZER LED_BUILTIN
#define BUTTON_0 0

#define DELAY 1024
#define SHORT_DELAY 128
#define LONG_DELAY 8192

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_0, INPUT);
  Serial.begin(115200);
  delay(SHORT_DELAY);

  Serial.println();
  Serial.println(F("******************************"));
  Serial.println(F("Prueba del BUZZER con Tickers"));
  Serial.println(F("para el monitorCO2 IER-UNAM"));
  Serial.println(F("(C) 2021, <hdcg@ier.unam.mx>"));
  Serial.println();
  
  Serial.println("Ticker...");
  tickerButton.attach_ms(DELAY, tckrButton);
  tickerBuzzer.attach_ms(SHORT_DELAY, tckrBuzzer);
}

volatile boolean isTime2button = true;
volatile boolean isTime2buzz = false;

void loop() {
  if(isTime2button) doButton();
  if(isTime2buzz) doBuzz();
}

void tckrButton(void) { isTime2button=true; }

void doButton(void) {
  isTime2button=false;
  byte value = digitalRead(BUTTON_0);
  if(value==LOW) {
    Serial.println("BUTTON_0 pressed");
    startAlarm(8);
  }
}

volatile byte buzzDuration = 0;

void startAlarm(byte duration) {
  if(duration==0) return;
  buzzDuration=duration;
}

void tckrBuzzer(void) {
  if(buzzDuration>0) {
    buzzDuration--;
    isTime2buzz=true;
  }
}

void doBuzz() {
  doBuzz2();
}

void doBuzz0(void) {
  isTime2buzz=false;
  boolean state=(buzzDuration&1)!=0;
  Serial.printf(PSTR("Buzzer is %s\n"), 
    state?PSTR("ON"):PSTR("OFF"));
  digitalWrite(BUZZER, state);
}
void doBuzz1(void) {
  isTime2buzz=false;
  boolean state=(buzzDuration&1)!=0;
  Serial.printf(PSTR("Buzzer is %s\n"), 
    state?PSTR("ON"):PSTR("OFF"));
  if(state) tone(BUZZER, state);
  else noTone(BUZZER);
}

void doBuzz2(void) {
  isTime2buzz=false;
  boolean state=(buzzDuration&1)!=0;
  Serial.printf(PSTR("Buzzer is %s\n"), 
    state?PSTR("ON"):PSTR("OFF"));
  if(state) tone(BUZZER, state, SHORT_DELAY);
}
