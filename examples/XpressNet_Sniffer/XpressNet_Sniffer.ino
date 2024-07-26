/*


  Simple XpressNet sniffer for Arduino MEGA or ESP8266/ESP32


*/

#include <XpressNet.h>

// -----------------------------------------------------------
// XpressNet settings
// -----------------------------------------------------------
// My XpressNet address
#define XNetAddress 30

#if defined(ESP8266) || defined(ESP32)
#define XNetPort D7			//Serial TX/RX 
#define XNet_TXRX D8		//send/receive pin
#else
#define XNet_TXRX 9		//send/receive pin	
#endif

XpressNetClass XpressNet;

void setup()
{
  // debug output
  Serial.begin(115200);
  Serial.println();
  Serial.println("XpressNet Sniffer");
  pinMode(LED_BUILTIN, OUTPUT);

  #if defined(ESP8266) || defined(ESP32)
	XpressNet.start(XNetAddress, XNetPort, XNet_TXRX);
  #else
	XpressNet.start(XNetAddress, XNet_TXRX);
  #endif
	  
  XpressNet.setPower(csNormal);

}

void loop()
{
  XpressNet.receive();
}

void notifyXNetStatus (uint8_t State)
{
  digitalWrite(LED_BUILTIN, State);
}

//--------------------------------------------------------------------------------------------
void notifyXNetPower (uint8_t State)
{
  Serial.print("Power: ");
  switch (State) {
    case csNormal: Serial.println("ON"); break;
    case csTrackVoltageOff: Serial.println("OFF"); break;
    case csEmergencyStop: Serial.println("EmStop"); break;
    case csShortCircuit: Serial.println("SHORT"); break;
    case csServiceMode: Serial.println("PROG"); break;
  }
}

//--------------------------------------------------------------------------------------------
void notifyXNetVersion(uint8_t Version, uint8_t ID ) 
{
  Serial.print("Version: ");
  Serial.println(Version, HEX);
}

//--------------------------------------------------------------------------------------------
void notifyLokAll(uint8_t Adr_High, uint8_t Adr_Low, boolean Busy, uint8_t Steps, uint8_t Speed, uint8_t Direction, uint8_t F0, uint8_t F1, uint8_t F2, uint8_t F3, boolean Req ) {
   Serial.print(Busy); 
   Serial.print(" Loco ALL: "); 
   Serial.print(Adr_Low); 
   Serial.print(", Steps: "); 
   Serial.print(Steps, BIN); 
   Serial.print(", Speed: "); 
   Serial.print(Speed); 
   Serial.print(", Direction: "); 
   Serial.print(Direction); 
   Serial.print(", Fkt: "); 
   Serial.print(F0, BIN); 
   Serial.print("; "); 
   Serial.print(F1, BIN); 
   Serial.print(", Fkt2: "); 
   Serial.print(F2, BIN); 
   Serial.print("; "); 
   Serial.println(F3, BIN); 
}

//--------------------------------------------------------------------------------------------
void notifyLokFunc(uint8_t Adr_High, uint8_t Adr_Low, uint8_t F2, uint8_t F3 ) {
  Serial.print("Loco Fkt: "); 
  Serial.print(Adr_Low); 
  Serial.print(", Fkt2: "); 
  Serial.print(F2, BIN); 
  Serial.print("; "); 
  Serial.println(F3, BIN); 
}

//--------------------------------------------------------------------------------------------
void notifyTrnt(uint8_t Adr_High, uint8_t Adr_Low, uint8_t Pos) {
  Serial.print("Turnout: "); 
  Serial.print(word(Adr_High, Adr_Low)); 
  Serial.print(", Position: "); 
  Serial.println(Pos, BIN); 
}

//--------------------------------------------------------------------------------------------
void notifyCVInfo(uint8_t State ) {
  Serial.print("CV Prog STATE: "); 
  Serial.println(State); 
}

//--------------------------------------------------------------------------------------------
void notifyCVResult(uint8_t cvAdr, uint8_t cvData ) {
  Serial.print("CV Prog Read: "); 
  Serial.print(cvAdr); 
  Serial.print(", "); 
  Serial.println(cvData); 
}
