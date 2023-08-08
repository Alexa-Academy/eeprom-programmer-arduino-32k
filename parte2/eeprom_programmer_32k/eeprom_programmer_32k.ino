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

enum SmState {
  STATE_START,
  STATE_GOT_AA,
  STATE_GOT_COMMAND,
  STATE_GOT_LENGHT_HIGH,
  STATE_GOT_LENGHT_LOW
};

byte state = STATE_START; // Stato corrente della macchina a stati

byte command; // Comando ricevuto

byte cmdLenghtHigh = 0;  // Byte alto della lunghezza espressa su due byte

word dataLenght = 0;  // Lunghezza dei dati da leggere o scrivere

word requestedDataToRead = 0; // Lunghezza dei dati da leggere (numero di byte)

word writeAddressIdx = 0;  // Indice del byte scritto in EEPROM

void sendResponse30();
void sendResponse31();

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

byte eepromBuffer[320];  // Dimensionato per leggere al più 20 banchi di memoria

void readMemoryBanks(int startBank, int numBanks) {
  int sBank = startBank;
  if (sBank > 2048) {
    sBank = 0;
  }

  int tBanks = numBanks;
  if (numBanks > 2048) {
    tBanks = 2048;
  }

  for (int bank=0; bank < tBanks; ++bank) {
    for (int i=0; i<16; ++i) {
      eepromBuffer[bank*16 + i] = readCycle((sBank+bank)*16 + i);
    }
  }
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
}

void sendResponse30() {
  byte message[324];

  if (requestedDataToRead > 0) {
    int numbBanks = ceil(requestedDataToRead / 16.0);

    int lenght = numbBanks * 16;

    message[0] = 0xAA;
    message[1] = 0x30;
    message[2] = (lenght & 0xFF00)>>8;
    message[3] = lenght & 0x00FF;

    int msgStart=4;

    int numPages = ceil(numbBanks / 20.0);

    for (int page=0; page<numPages; ++page) {
      int banksToRead = 20;
      if (page == numPages-1) {
        banksToRead = numbBanks - page*20;
      }
      readMemoryBanks(page * 20, banksToRead);

      for (int i=0; i<banksToRead*16; ++i) {
        message[msgStart+i] = eepromBuffer[i];
      }

      Serial.write(message, sizeof(message));
      Serial.flush();

      msgStart=0;
    }
  } else {
    message[0] = 0xAA;
    message[1] = 0x30;
    message[2] = 0x00;
    message[3] = 0x00;

    Serial.write(message, sizeof(message));
	  Serial.flush();
  }
}

void sendResponse31() {
  byte message[] = {0xAA, 0x31, (writeAddressIdx & 0xFF00)>>8, writeAddressIdx & 0x00FF};

	Serial.write(message, sizeof(message));
	Serial.flush();
}

void loopSerial() {
	if (Serial.available() > 0) {
		byte byteRead = Serial.read();
   // Serial.println(String(byteRead, HEX));
		
		if (state == STATE_START && byteRead == 0xAA) {
			state = STATE_GOT_AA;
		} else if (state == STATE_GOT_AA) {
			state = STATE_GOT_COMMAND;
			command = byteRead;
		} else if (state == STATE_GOT_COMMAND) {
			state = STATE_GOT_LENGHT_HIGH;
      cmdLenghtHigh = byteRead;
		} else if (state == STATE_GOT_LENGHT_HIGH) {
      dataLenght = (cmdLenghtHigh<<8) + byteRead;

      state = STATE_GOT_LENGHT_LOW;

      if (command == 0x30) {
        requestedDataToRead = dataLenght;
        sendResponse30();
        state = STATE_START;
			} else if (command == 0x31) {
        writeAddressIdx = 0;
      }
    } else if (state == STATE_GOT_LENGHT_LOW) {
      if (command == 0x31) {
        writeCycle(byteRead, writeAddressIdx++);

        if (writeAddressIdx == dataLenght) {
          sendResponse31();
          state = STATE_START;
        }
      } else {
				state = STATE_START;
			}
    }
	}
}

void loop() {
  loopSerial();
}

