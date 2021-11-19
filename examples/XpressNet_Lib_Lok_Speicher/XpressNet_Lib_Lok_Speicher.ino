/*
  XpressNet for Arduino

  Reset the last loco status on startup using the XpressNet Interface

  - Store the Lok Information in EEPROM
  - Restore the Information on startup
  - read all data that is send on the XpressNet
  
*/

#include <XpressNet.h>
XpressNetClass XpressNet;

#include <EEPROM.h>

// XpressNet address: must be in range of 1-31; must be unique. Note that some IDs
// are currently used by default, like 2 for a LH90 or LH100 out of the box, or 30
// for PC interface devices like the XnTCP.
#define XNetAddress 30 //Adresse im XpressNet

#define MAXLOK 100 //  (100 * 10 = 1000) + 10 Byte overhead! -> max EEPROM 1024 Byte 
#define refreshTime 10 //ms to wait between each packet

#define ledPIN 13   //XpressNet Status
#define deletePIN 12 //Remove all Information!

//Debugging:
//#define DEBUG Serial   

uint16_t LokAdr[MAXLOK];

void setup() {
  #if defined(DEBUG)
  DEBUG.begin(115200);
  DEBUG.println("XPressNet Lok store");
  #endif

  pinMode(ledPIN, OUTPUT);
  pinMode(deletePIN, INPUT_PULLUP);
  
  EEPROMReadStore();

  XpressNet.start(XNetAddress, 9);    //initialisierung XNet Serial und Send/Receive-PIN as SLAVE

  //XpressNet.setPower(0);
}

void loop() {
  
 XpressNet.receive(); // XpressNet work function: read and write to X-net bus.

 if (digitalRead(deletePIN) == LOW) {
    clearAllInfo();
 }

}

void EEPROMReadStore() {
  uint16_t adr = 0xFFFF;
  for (uint16_t Slot = 0; Slot < MAXLOK; Slot++) {
    adr = EEPROM.read(Slot*10) | (EEPROM.read(Slot*10+1) << 8);
    if (adr != 0xFFFF && adr <= 9999) {
      for (uint16_t x = 0; x < Slot; x++) {
        //doppelte Einträge?
        if (LokAdr[x] == adr) {
          #if defined(DEBUG)  
          DEBUG.print("löschen ");
          #endif
          for (byte del = 0; del < 10; del++) {
            EEPROM.update(Slot*10+del, 0xFF);
          }
        }
      }
      LokAdr[Slot] = adr;
      #if defined(DEBUG)
      DEBUG.print("Lok: ");
      DEBUG.print(adr);
      #endif
      //speed:
      if (EEPROM.read(Slot*10+3) != 0xFF) {
        XpressNet.setLocoDrive(highByte(adr), lowByte(adr), EEPROM.read(Slot*10+2) /*Steps*/, EEPROM.read(Slot*10+3) /*Speed*/); 
        #if defined(DEBUG)
        DEBUG.print(" S");
        DEBUG.print(EEPROM.read(Slot*10+2));
        DEBUG.print(":");
        DEBUG.print(EEPROM.read(Slot*10+3));
        #endif
        delay(refreshTime);
      }
      if (EEPROM.read(Slot*10+4) <= 0x1F && EEPROM.read(Slot*10+4) != 0) {
        XpressNet.setFunc0to4(adr, EEPROM.read(Slot*10+4));
        #if defined(DEBUG)
        DEBUG.print(" F0:");
        DEBUG.print(EEPROM.read(Slot*10+4), BIN);
        #endif
        delay(refreshTime);
      }
      if (EEPROM.read(Slot*10+5) <= 0x0F && EEPROM.read(Slot*10+5) != 0) {
        XpressNet.setFunc5to8(adr, EEPROM.read(Slot*10+5));  
        #if defined(DEBUG)
        DEBUG.print(" F1:");
        DEBUG.print(EEPROM.read(Slot*10+5), BIN);
        #endif
        delay(refreshTime);
      }
      if (EEPROM.read(Slot*10+6) <= 0x0F && EEPROM.read(Slot*10+6) != 0) {
        XpressNet.setFunc9to12(adr, EEPROM.read(Slot*10+6));  
        #if defined(DEBUG)
        DEBUG.print(" F2:");
        DEBUG.print(EEPROM.read(Slot*10+6), BIN);
        #endif
        delay(refreshTime);
      }
      if (EEPROM.read(Slot*10+7) != 0) { 
        XpressNet.setFunc13to20(adr, EEPROM.read(Slot*10+7));
        #if defined(DEBUG)
        DEBUG.print(" F3:");
        DEBUG.print(EEPROM.read(Slot*10+7), BIN);
        #endif
        delay(refreshTime);
      }
      if (EEPROM.read(Slot*10+8) != 0) {
        XpressNet.setFunc21to28(adr, EEPROM.read(Slot*10+8));  
        #if defined(DEBUG)  
        DEBUG.print(" F4:");
        DEBUG.print(EEPROM.read(Slot*10+8), BIN);
        #endif
        delay(refreshTime);
      }
      #if defined(DEBUG)  
      DEBUG.println();
      #endif
    }
    else {
      //leeren:
      LokAdr[Slot] = 0xFFFF;
      for (byte del = 0; del < 10; del++) {
        EEPROM.update(Slot*10+del, 0xFF);
      }
    }
  }
}

uint16_t getSlot(uint16_t adr) {
  for (uint16_t Slot = 0; Slot < MAXLOK; Slot++) {
    if (LokAdr[Slot] == adr)
      return Slot; //vorhanden
  }
  //neu anlegen:
  for (uint16_t Slot = 0; Slot < MAXLOK; Slot++) {
    if (LokAdr[Slot] == 0xFFFF) {
      LokAdr[Slot] = adr;
      EEPROM.update(Slot*10,adr & 0xFF);
      EEPROM.update(Slot*10+1, adr >> 8);
      EEPROM.update(Slot*10+4, 0);
      EEPROM.update(Slot*10+5, 0);
      EEPROM.update(Slot*10+6, 0);
      EEPROM.update(Slot*10+7, 0);
      EEPROM.update(Slot*10+8, 0);
      #if defined(DEBUG)
      DEBUG.print("Neue Lok ");
      DEBUG.println(adr);
      #endif
      XpressNet.getLocoInfo(adr >> 8, adr);
      return Slot;
    }
  }
  return 0;
}

//--------------------------------------------------------------------------------------------
void notifyXNetPower(uint8_t State) {
  #if defined(DEBUG)
  DEBUG.print("Power: ");
  DEBUG.println(State, HEX);
  #endif
}
/*
//--------------------------------------------------------------------------------------------
void notifyLokFunc(uint8_t Adr_High, uint8_t Adr_Low, uint8_t F2, uint8_t F3)
{
  #if defined(DEBUG)
  DEBUG.println("Loco Fkt: ");
  DEBUG.println(Adr_Low);
  DEBUG.println(", Fkt2: ");
  DEBUG.println(F2, BIN);
  DEBUG.println("; ");
  DEBUG.println(F3, BIN);
  #endif
}
*/
//--------------------------------------------------------------------------------------------
void notifyLokAll(uint8_t Adr_High, uint8_t Adr_Low, boolean Busy, uint8_t Steps, uint8_t Speed, uint8_t Direction, uint8_t F0, uint8_t F1, uint8_t F2, uint8_t F3, boolean Req)
{
  #if defined(DEBUG)
  DEBUG.print("notifyLokAll(): ADDR_HI: ");
  DEBUG.print(Adr_High, DEC);
  DEBUG.print(", ADDR_LO: ");
  DEBUG.print(Adr_Low, DEC);
  DEBUG.print(", STEPS: ");
  DEBUG.print(Steps, DEC);
  DEBUG.print(", Speed: ");
  DEBUG.print(Speed, BIN);
  DEBUG.print(", F1-4: ");
  DEBUG.print(F0, BIN);
  DEBUG.print(", F5-12: ");
  DEBUG.print(F1, BIN);
  DEBUG.print(", F13-20: ");
  DEBUG.print(F2, BIN);
  DEBUG.print(", F21-28: ");
  DEBUG.println(F3, BIN);
  #endif
  
  uint16_t Slot = getSlot(word(Adr_High,Adr_Low));
  EEPROM.update(Slot*10, Adr_Low);
  EEPROM.update(Slot*10+1, Adr_High);
  if (Steps != 0)
    EEPROM.update(Slot*10+2, Steps);
  EEPROM.update(Slot*10+3, Speed);
  EEPROM.update(Slot*10+4, F0);
  EEPROM.update(Slot*10+5, F1 & 0x0F);
  EEPROM.update(Slot*10+6, F1 >> 4);
  EEPROM.update(Slot*10+7, F2);
  EEPROM.update(Slot*10+8, F3);

  //  XpressNet.getLocoInfo(Adr_High, Adr_Low);
}

/*
//--------------------------------------------------------------------------------------------
void notifyTrnt(uint8_t Adr_High, uint8_t Adr_Low, uint8_t Pos)
{
  #if defined(DEBUG)
  DEBUG.print("TurnOut: ");
  DEBUG.print(word(Adr_High, Adr_Low));
  DEBUG.print(", Position: ");
  DEBUG.println(Pos, BIN);
  #endif
}
//--------------------------------------------------------------------------------------------
void notifyCVInfo(uint8_t State)
{
}
//--------------------------------------------------------------------------------------------
void notifyCVResult(uint8_t cvAdr, uint8_t cvData)
{
}
//--------------------------------------------------------------------------------------------
void notifyXNetVersion(uint8_t Version, uint8_t ID)
{
}
*/
//--------------------------------------------------------------------------------------------
void notifyXNetStatus(uint8_t LedState)
{
  digitalWrite(ledPIN, LedState);
}

//--------------------------------------------------------------------------------------------
void clearAllInfo() {
  for (uint16_t Slot = 0; Slot < MAXLOK; Slot++) {
    LokAdr[Slot] = 0xFFFF;
    EEPROM.update(Slot*10, 0xFF);
    EEPROM.update(Slot*10+1, 0xFF);
    EEPROM.update(Slot*10+2, 0);
    EEPROM.update(Slot*10+3, 0);
    EEPROM.update(Slot*10+4, 0);   //f0..f4
    EEPROM.update(Slot*10+5, 0);  //f5..f8
    EEPROM.update(Slot*10+6, 0);  //f9..12
    EEPROM.update(Slot*10+7, 0);   //f13..f20
    EEPROM.update(Slot*10+8, 0);  //f21..f28
  }
  #if defined(DEBUG)
  DEBUG.println("CLEAR ALL!");
  #endif
}
