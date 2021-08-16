#ifndef ARDUINO_ARCH_ESP8266
#error Requires NodeMCU esp8266 board
#endif 
#define DEBUG
/*
  proyecto monitorCO2

  prueba del EEPROM para grabar la configuración
  individual de cada sensor (sic)
    
  (C) 2021 Héctor Daniel Cortés González <hdcg@ier.unam.mx>
  (C) 2021 Instituto de Energías Renovables <www.ier.unam.mx>
  (C) 2021 Universidad Nacional Autónoma de México <www.unam.mx>
*/
#include <EEPROM.h>
#include <CRC8.h>

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
} myEEPROM;

myEEPROM::myEEPROM() {}
myEEPROM::~myEEPROM() {}

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

String mkStr(const byte size) {
  String retval;
  for(byte i=0; i<size; i++) 
    retval+=random('A','Z');  
  return retval;
}

eeprom_data_t *data;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  randomSeed(0xC0CAC07A);
  Serial.println("EEPROM test...");
  data = new eeprom_data_t;
//  Serial.setTimeout(TIMEOUT);
}

String getString() {
  String retval;
  while(1) {
    while(!Serial.available());
    char ch = Serial.read();
    if(ch=='\n') break;
    retval+=ch;
  }
  return retval;
}

void loop() {
  String cmd;

  Serial.print("ACCESS_TOKEN=");
  Serial.println(data->access_token);
  Serial.print("WIFI_SSID=");
  Serial.println(data->wifi_ssid);
  Serial.print("WIFI_PASS=");
  Serial.println(data->wifi_pass);

  Serial.print("\nEEPROM: ?");
  cmd=getString();
  Serial.println(cmd);

  if(cmd==String("read")) {
    if(myEEPROM.readData(data))
      Serial.println("EEPROM read OK");
    else
      Serial.println("EEPROM read error");
  }
  if(cmd=="write") {
    if(myEEPROM.writeData(data))
      Serial.println("EEPROM write OK");
    else
      Serial.println("EEPROM write error");
  }
  if(cmd=="dump") {
    myEEPROM.dump();
  }
  if(cmd=="access_token") {
    Serial.print("access_token: ?");
    data->access_token = getString();
  }
  if(cmd=="wifi_ssid") {
    Serial.print("wifi_ssid: ?");
    data->wifi_ssid = getString();
  }
  if(cmd=="wifi_pass") {
    Serial.print("Enter wifi_pass: ?");
    data->wifi_pass = getString();
  }
}
