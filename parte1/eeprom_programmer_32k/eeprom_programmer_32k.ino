#include "Arduino.h"

#define MEM_IO1  22
#define MEM_IO2  24
#define MEM_IO3  26
#define MEM_IO4  28
#define MEM_IO5  30
#define MEM_IO6  32
#define MEM_IO7  34
#define MEM_IO8  36

#define MEM_A0  23
#define MEM_A1  25
#define MEM_A2  27
#define MEM_A3  29
#define MEM_A4  31
#define MEM_A5  33
#define MEM_A6  35
#define MEM_A7  37
#define MEM_A8  39
#define MEM_A9  41
#define MEM_A10  43
#define MEM_A11  45
#define MEM_A12  47
#define MEM_A13  49
#define MEM_A14  51

#define MEM_WE  50
#define MEM_OE  46
#define MEM_CS  48

void setDatabusOut(bool isOut) {
  if (isOut) {
    pinMode(MEM_IO1, OUTPUT);
    pinMode(MEM_IO2, OUTPUT);
    pinMode(MEM_IO3, OUTPUT);
    pinMode(MEM_IO4, OUTPUT);
    pinMode(MEM_IO5, OUTPUT);
    pinMode(MEM_IO6, OUTPUT);
    pinMode(MEM_IO7, OUTPUT);
    pinMode(MEM_IO8, OUTPUT);
  } else {
    pinMode(MEM_IO1, INPUT);
    pinMode(MEM_IO2, INPUT);
    pinMode(MEM_IO3, INPUT);
    pinMode(MEM_IO4, INPUT);
    pinMode(MEM_IO5, INPUT);
    pinMode(MEM_IO6, INPUT);
    pinMode(MEM_IO7, INPUT);
    pinMode(MEM_IO8, INPUT);
  }
}

byte readData() {
  bool d7 = digitalRead(MEM_IO8);
  bool d6 = digitalRead(MEM_IO7);
  bool d5 = digitalRead(MEM_IO6);
  bool d4 = digitalRead(MEM_IO5);
  bool d3 = digitalRead(MEM_IO4);
  bool d2 = digitalRead(MEM_IO3);
  bool d1 = digitalRead(MEM_IO2);
  bool d0 = digitalRead(MEM_IO1);

  byte data_bus = d7<<7 | d6<<6 | d5<<5 | d4<<4 | d3<<3 | d2<<2 | d1<<1 | d0;

  return data_bus;
}

void writeData(byte b) {
  digitalWrite(MEM_IO1, bitRead(b, 0));
  digitalWrite(MEM_IO2, bitRead(b, 1));
  digitalWrite(MEM_IO3, bitRead(b, 2));
  digitalWrite(MEM_IO4, bitRead(b, 3));
  digitalWrite(MEM_IO5, bitRead(b, 4));
  digitalWrite(MEM_IO6, bitRead(b, 5));
  digitalWrite(MEM_IO7, bitRead(b, 6));
  digitalWrite(MEM_IO8, bitRead(b, 7));
}

void writeAddress(word w) {
  digitalWrite(MEM_A0, bitRead(w, 0));
  digitalWrite(MEM_A1, bitRead(w, 1));
  digitalWrite(MEM_A2, bitRead(w, 2));
  digitalWrite(MEM_A3, bitRead(w, 3));
  digitalWrite(MEM_A4, bitRead(w, 4));
  digitalWrite(MEM_A5, bitRead(w, 5));
  digitalWrite(MEM_A6, bitRead(w, 6));
  digitalWrite(MEM_A7, bitRead(w, 7));
  digitalWrite(MEM_A8, bitRead(w, 8));
  digitalWrite(MEM_A9, bitRead(w, 9));
  digitalWrite(MEM_A10, bitRead(w, 10));
  digitalWrite(MEM_A11, bitRead(w, 11));
  digitalWrite(MEM_A12, bitRead(w, 12));
  digitalWrite(MEM_A13, bitRead(w, 13));
  digitalWrite(MEM_A14, bitRead(w, 14));
}

void writeCycle(byte data, word address) {
  writeAddress(address);
  delayMicroseconds(3);
  digitalWrite(MEM_OE, 1);
  delayMicroseconds(3);
  digitalWrite(MEM_CS, 0);
  delayMicroseconds(3);
  setDatabusOut(true);
  writeData(data);
  delayMicroseconds(3);
  digitalWrite(MEM_WE, 0);
  delayMicroseconds(1);
  digitalWrite(MEM_WE, 1);
  delayMicroseconds(3);
  setDatabusOut(false);
  delayMicroseconds(3);
  digitalWrite(MEM_CS, 1);
  delay(6);
}

byte readCycle(word address) {
  writeAddress(address);
  digitalWrite(MEM_CS, 0);
  delayMicroseconds(3);
  digitalWrite(MEM_OE, 0);
  delayMicroseconds(3);
  byte data = readData();
  digitalWrite(MEM_OE, 1);
  delayMicroseconds(3);
  digitalWrite(MEM_CS, 1);
  delayMicroseconds(3);

  return data;
}

char buffer[1000];

void readMemoryBanks(int numBanks) {
  int tBanks = numBanks;
  if (numBanks > 2048) {
    tBanks = 2048;
  }

  byte data_read[8];
  char data_read_ascii[8];

  for (int bank=0; bank < tBanks; ++bank) {
    for (int i=0; i<8; ++i) {      
      data_read[i] = readCycle(bank*8 + i);
      if(isprint(data_read[i])) {
        data_read_ascii[i] = data_read[i];
      } else {
        data_read_ascii[i] = '.';
      }
    }

    sprintf(buffer, "%04X: %02X %02X %02X %02X %02X %02X %02X %02X %c%c%c%c%c%c%c%c", bank*8, data_read[0], data_read[1], data_read[2], data_read[3], data_read[4], data_read[5], data_read[6], data_read[7], data_read_ascii[0], data_read_ascii[1], data_read_ascii[2], data_read_ascii[3], data_read_ascii[4], data_read_ascii[5], data_read_ascii[6], data_read_ascii[7]);

    Serial.println(buffer);
  }
}

void programEPROM() {
  byte ROM[] = {0x3E, 0x00, 0xD3, 0x06, 0xAF, 0xFE, 0x0A, 0x30, 0x03, 0x3C, 0x18, 0xF9, 0x3E, 0x01, 0xD3, 0x06, 0x01, 0x00, 0x00, 0x79, 0xD6, 0x0A, 0x78, 0x17, 0x3F, 0x1F, 0xDE, 0x80, 0x30, 0xE2, 0x03, 0x18, 0xF2};

  for (int i=0; i<sizeof(ROM); ++i) {
    writeCycle(ROM[i], i);
  }

  delay(10);
}

void setup() {
  Serial.begin(115200);

  setDatabusOut(false);

  pinMode(MEM_A0, OUTPUT);
  pinMode(MEM_A1, OUTPUT);
  pinMode(MEM_A2, OUTPUT);
  pinMode(MEM_A3, OUTPUT);
  pinMode(MEM_A4, OUTPUT);
  pinMode(MEM_A5, OUTPUT);
  pinMode(MEM_A6, OUTPUT);
  pinMode(MEM_A7, OUTPUT);
  pinMode(MEM_A8, OUTPUT);
  pinMode(MEM_A9, OUTPUT);
  pinMode(MEM_A10, OUTPUT);
  pinMode(MEM_A11, OUTPUT);
  pinMode(MEM_A12, OUTPUT);
  pinMode(MEM_A13, OUTPUT);
  pinMode(MEM_A14, OUTPUT);
  
  pinMode(MEM_WE, OUTPUT);
  pinMode(MEM_OE, OUTPUT);
  pinMode(MEM_CS, OUTPUT);

  digitalWrite(MEM_WE, 1);
  digitalWrite(MEM_OE, 1);
  digitalWrite(MEM_CS, 1);

  //programEPROM();

  readMemoryBanks(5);
}

void loop() {
}

