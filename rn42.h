#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include "KeyReport.h"

// RN-42 consumer codes
#define RN_RELEASE 0x0, 0x0        // Release consumer key
#define RN_HOME 0x1, 0x00          // AC Home
#define RN_EMAIL 0x2, 0x00         // AC Email Reader
#define RN_SEARCH 0x4, 0x00        // AC Search
#define RN_KEYBLAY 0x8, 0x00       // AL Keyboard Layout (Virtual Apple Keyboard Toggle)
#define RN_VOL_UP 0x10, 0x00       // Volume Up
#define RN_VOL_DOWN 0x20, 0x00     // Volume Down
#define RN_MUTE 0x40, 0x00         // Mute
#define RN_PLAY 0x80, 0x00         // Play/Pause
#define RN_NEXT 0x00, 0x01         // Scan Next Track
#define RN_PREV 0x00, 0x02         // Scan Previous Track
#define RN_STOP 0x00, 0x04         // Stop
#define RN_EJECT 0x00, 0x08        // Eject
#define RN_FORWARD 0x00, 0x10      // Fast Forward
#define RN_REWIND 0x00, 0x20       // Rewind
#define RN_STOP_EJECT 0x00, 0x40   // Stop/Eject
#define RN_BROWSER 0x00, 0x80      // AL Internet Browse

template <typename T>
class RN42{
 	public:
  void init(T &out, uint8_t state_pin) {
  	analogWrite(state_pin, 255);
  	if (out.available()) out.end();
  	delay(100);
  	out.begin(115200);
  	delay(320);                     // IMPORTANT DELAY! (Minimum ~276ms)
  	//out.print("$$$");         // Enter command mode
  	//delay(15);
  	//out.print("CFR\n");                 
  	//delay(100);
	}

  void close(T &out, uint8_t state_pin) {
  	digitalWrite(state_pin, LOW);
  	if (out.available()) out.end();
	}
 
  void WriteConsumer(T &out, byte low, byte high) {
  	out.write((byte)0xFD); //Start HID Report 
  	out.write((byte)0x3); //Length byte 
  	out.write((byte)0x3); //Descriptor byte 
  	out.write(low); 
  	out.write(high);
  	delay(15);
  	out.write((byte)0xFD); //Start HID Report 
  	out.write((byte)0x3); //Length byte 
  	out.write((byte)0x3); //Descriptor byte 
  	out.write((byte)0x00); 
  	out.write((byte)0x00);
  	return;
	}

 uint8_t getLeds(T &out, uint8_t old_state){
  uint8_t new_state;
  new_state = out.read();
  if (new_state == 0xFF){
    new_state = out.read();
  }
  return old_state;
 }

  void SendReport(T &out, KeyReport *report) {
  	out.write((byte)0xFD); //Start HID Report 
  	out.write((byte)0x9); //Length byte 
  	out.write((byte)0x1); //Descriptor byte 
  	out.write(report->get_modifiers()); //Modifier byte 
  	out.write((byte)0x00); //- 
  	for(byte i = 0; i < 6; i++) out.write((byte)(report->get_key(i))); 
	}
};
