/*
 * XpressNet Sniffer
 * Copyright (c) 04/2017 - 2017 Philipp Gahtow  All right reserved.
 * 
 * connect the MAX485 on Serial 1
 * connect MAX485 RE/DE to Pin 9 - or set it fix to "GND" => 'read'
 * 
 * only for Arduino MEGA!
*/

#define bufferlen 120 //Buffer for Read Data

//ISR read Data:
uint8_t len = 0;      //Zeiger Read Data
boolean dataread = false; //read a Callbyte 
uint16_t data[bufferlen];  //zu sendende Daten
uint16_t callbyte = 0;  //Save Read Callbyte

//Data dump for Output:
uint8_t lenout = 0;   //Zeiger Write Data
uint8_t startbyte = 0;   //start data packet

void setup() {
  //Set up on 62500 Baud
  cli();  //disable interrupts while initializing the USART
  //Serial0 on Arduino UNO
  #ifdef __AVR_ATmega8__
   UBRRH = 0;
   UBRRL = 0x0F;
   UCSRA = 0;
   UCSRB = (1<<RXEN) | (1<<TXEN) | (1<<RXCIE) | (1<<UCSZ2);
   UCSRC = (1<<UCSZ1) | (1<<UCSZ0);
  #else
  //Serial1
   UBRR1H = 0;
   UBRR1L = 0x0F;
   UCSR1A = 0;
   UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1<<RXCIE1) | (1<<UCSZ12);
   UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);
   #endif
  sei(); // Enable interrupts
  
  Serial.begin(115200);
  Serial.println("XpressNet Sniffer");
  //set MAX485 into 'read' Mode!
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
}

void loop() {

  //E3 F0 HAdr LAdr CRC
  //E7 40 (Dir,Speed) (Busy,F0,F4,F3,F2,F1) F5-F12 F13-F20 0x00 0x00 CRC

  if (len != lenout) {     // If anything comes in Serial1 (pins 0 & 1)
    if (data[lenout] >= 0x100) { //neues Call Byte
      //Auswertung des letzten Packets:
      switch (data[startbyte+1]) { //get the Header Information
        case 0x42: Serial.print(" AccInfo"); break;
        case 0x61: Serial.print(" Power"); break;
        case 0x81: Serial.print(" Power"); break;
        case 0x63: switch (data[startbyte+2]) {
                      case 0x21: Serial.print(" Version"); break;
                      default: Serial.print(" P-Info"); 
                    }
                   break;
        case 0xE3: switch (data[startbyte+2]) {
                     case 0x00: Serial.print(" REQ LokData"); break;
                     case 0x07: Serial.print(" REQ FktStat"); break;
                     case 0x08: Serial.print(" REQ FktStat13-28"); break;
                     case 0x09: Serial.print(" REQ FktData"); break;
                     case 0x40: Serial.print(" Busy"); break;
                     case 0x50: Serial.print(" FktStatus"); break;
                     case 0x51: Serial.print(" FktStatus13-28"); break;
                     case 0x52: Serial.print(" FktData13-28"); break;
                     case 0xF0: Serial.print(" REQ LocoDataMM"); break;
                    }
                    break;
        case 0xE4: Serial.print(" LokInfo"); break;             
        case 0xE7: Serial.print(" LocoInfo MM"); break;
      }
      //Starte neues Packet:
      Serial.println();
      startbyte = lenout;  //mark as first sector packet (CallByte)
      //Decode CallByte:
      switch ((data[lenout] >> 5) & 0x03) {
         case 0:  //Request ACK
                 Serial.print("A-");
                 break;
         case 2: //Inquiry
                  Serial.print("I-");
                  break;
          default: //DirectedOps
                  Serial.print("D-"); 
      }
      Serial.print(data[lenout] & 0x1F);
      Serial.print(" Slot:");
    }
    //write out normal data byte
    if (data[lenout] < 0x0A)
      Serial.print(" 0x0");  //führende '0' voranstellen
    else Serial.print(" 0x");
    Serial.print(data[lenout], HEX);

    lenout++; //get the next byte
    if (lenout == bufferlen)
      lenout = 0;
  }
}

//--------------------------------------------------------------------------------------------
//UART interrupt handler 
ISR(USART1_RX_vect) {
  unsigned char status, resh, resl;
  // Wait for data to be received

  status = UCSR1A;
  while (!(status & (1 << RXC1))) { return; }//status = UCSR1A;}

  // Get status and 9th bit, then data 
  resh = UCSR1B;
  resl = UDR1;

  // If error, return -1
  if (status & ((1 << FE1) | (1 << DOR1) | (1 << UPE1))) { return; }

  // Filter the 9th bit, then return 
  resh = (resh >> 1) & 0x01;
  int dat = ((resh << 8) | resl); //Save the read data

  //Check if data is a CallByte?
  if (dat >= 0x100) {
    callbyte = dat;   //store it
    dataread = true;
  }
  else {  //read normal data
    if (dataread)  {  //first add callbyte to buffer
      data[len] = callbyte;
      dataread = false; //mark it for write
      len++;
      if (len == bufferlen)
        len = 0;  
    }
    data[len] = dat;  //add normal data to buffer
    len++;
    if (len == bufferlen)
      len = 0;
  }
}
