#include "myEEPROM.h"

myEEPROM::myEEPROM() { EEPROM.begin(512); }
myEEPROM::~myEEPROM() { EEPROM.end(); }

char myEEPROM::readChar() {
  char ch;
  ch=EEPROM.read(address++);
  crc.add(ch);
  return ch;
}

char myEEPROM::writeChar(const char ch) {
  crc.add(ch);
  EEPROM.write(address++, ch);
  return ch;
}

char myEEPROM::readCRC() {
  return EEPROM.read(511);
}

char myEEPROM::writeCRC() {
  char ch = crc.getCRC();
  EEPROM.write(511, ch);
  return ch;
}

String myEEPROM::readString() {
  String retval;
  byte len=readChar();
  for(byte i=0; i<len; i++)
    retval += readChar();
  return retval;
}

String myEEPROM::writeString(const String S) {
  byte len=S.length();
  writeChar(len);
  for(byte i=0; i<len; i++)
    writeChar(S.charAt(i));
  return S;
}

boolean myEEPROM::readData(eeprom_data_t *p) {
// RESET
  crc.reset(); 
  address=0;
// MAGIC NUMBER
  if(readChar()!='X') return false;
  if(readChar()!='G') return false;
// REV number
  if(readChar()!=REV) return false;
// AQUI COMIENZA LO BUENO
  p->access_token = readString();
  p->wifi_ssid = readString();
  p->wifi_pass = readString();
// READ CRC
  if(EEPROM.read(511)!=crc.getCRC()) return false;
  return true;
}

boolean myEEPROM::writeData(eeprom_data_t *p) {
// RESET
  crc.reset();
  address=0;
// Write MAGIC number
  writeChar('X');
  writeChar('G');
// Write REV number
  writeChar(REV);
// AQUI EMPIEZA LO BUENO
  writeString(p->access_token);
  writeString(p->wifi_ssid);
  writeString(p->wifi_pass);
// WRITE CRC
  writeCRC();
// COMMIT  
  return EEPROM.commit();
}

boolean myEEPROM::clear(const char ch) {
  for(address=0; address<512; address++)
    EEPROM.write(address, ch);
  return EEPROM.commit();
}

void myEEPROM::dump() {
  char ch;
  for(address=0; address<512; address++) {
    if((address&0x0f)==0) Serial.printf("%04x ", address);
    ch=EEPROM.read(address);
    if(ch<16) Serial.print("0");
    Serial.print(ch, HEX);
    if((address&0x0f)==0x0f) Serial.println();
  }
}
