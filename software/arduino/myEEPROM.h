#ifndef _myEEPROM_H_
#define _myEEPROM_H_

#include <EEPROM.h>
#include <CRC8.h>

//#include <Arduino.h>

#define MAGIC "XG"
#define REV 0xff
#define TIMEOUT (1*60*1000)

typedef struct {
  String access_token;
  String wifi_ssid;
  String wifi_pass;
} eeprom_data_t;

class myEEPROM {
  public:
    myEEPROM();
    ~myEEPROM();
    boolean readData(eeprom_data_t *p);
    boolean writeData(eeprom_data_t *p);
    boolean clear(const char ch=0);
    void dump();
  protected:
    CRC8 crc;
    unsigned short address;
    char readChar();
    char writeChar(const char ch);
    String readString();
    String writeString(const String S);
    char readCRC();
    char writeCRC();
};

#endif
