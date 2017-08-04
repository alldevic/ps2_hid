#include "ps2_Keyboard.h"
#include "ps2_NullDiagnostics.h"
#include "ps2_UsbTranslator.h"
#include "HID-Project.h"

static bool is_usb = true;
static ps2::NullDiagnostics diagnostics;
static ps2::UsbTranslator<ps2::NullDiagnostics> keyMapping(diagnostics);
static ps2::Keyboard<2,3,16, ps2::NullDiagnostics> ps2Keyboard(diagnostics);
static ps2::UsbKeyboardLeds ledValueLastSentToPs2 = ps2::UsbKeyboardLeds::none;

void BT_init(){
  analogWrite(4, 255);
  if (Serial1.available()) Serial1.end();
  Serial1.begin(115200);
  delay(320);                     // IMPORTANT DELAY! (Minimum ~276ms)
  //Serial1.print("$$$");         // Enter command mode
  //delay(15);
  //Serial1.print("CFI\n");                 
  //delay(100);
}

void BT_close(){
  digitalWrite(4, LOW);
  if (Serial1.available()) Serial1.end();
}

void BT_SendConsumer(byte consumer){
  Serial1.write((byte)0xFD); //Start HID Report 
  Serial1.write((byte)0x3); //Length byte 
  Serial1.write((byte)0x3); //Descriptor byte 
  Serial1.write(consumer); 
  Serial1.write((byte)0x0);
}

//void BT_SendReport(KeyReport *report){
//  Serial1.write((byte)0xFD); //Start HID Report 
//  Serial1.write((byte)0x9); //Length byte 
//  Serial1.write((byte)0x1); //Descriptor byte 
//  Serial1.write(report->modifiers); //Modifier byte 
//  Serial1.write((byte)0x00); //- 
//  for(byte i = 0; i < 6; i++) Serial1.write((byte)(report->keys[i])); 
//}
 
void setup() {
  analogWrite(4, 0);
  if (!is_usb) BT_init();
  ps2Keyboard.begin();
  Consumer.begin();
  BootKeyboard.begin();
}

static bool fn = false;

void loop() {
    ps2::UsbKeyboardLeds newLedState = (ps2::UsbKeyboardLeds)BootKeyboard.getLeds();
    if (newLedState != ledValueLastSentToPs2) {
        ps2Keyboard.sendLedStatus(keyMapping.translateLeds(newLedState));
        ledValueLastSentToPs2 = newLedState;
    }

    ps2::KeyboardOutput scanCode = ps2Keyboard.readScanCode();
    if (scanCode == ps2::KeyboardOutput::garbled) {}
    else if (scanCode != ps2::KeyboardOutput::none) {
        ps2::UsbKeyAction action = keyMapping.translatePs2Keycode(scanCode);
        KeyboardKeycode hidCode = (KeyboardKeycode)action.hidCode;
               
        switch (action.gesture) {
            case ps2::UsbKeyAction::KeyDown:
              if (hidCode == KEY_RIGHT_ALT) fn = true;
              if (fn){
                if (hidCode == KEY_DOWN) Consumer.write(MEDIA_PLAY_PAUSE);  
                if (hidCode == KEY_RIGHT) Consumer.write(MEDIA_NEXT);  
                if (hidCode == KEY_LEFT) Consumer.write(MEDIA_PREV);  
                if (hidCode == KEY_UP) Consumer.write(MEDIA_STOP), fn = false;  
                if (hidCode == KEY_F10) Consumer.write(MEDIA_VOL_MUTE), fn = false;  
                if (hidCode == KEY_F11) Consumer.write(MEDIA_VOL_DOWN);  
                if (hidCode == KEY_F12) Consumer.write(MEDIA_VOL_UP);
                if (hidCode == KEY_F8) Consumer.write(CONSUMER_CALCULATOR), fn = false;
                if (hidCode == KEY_F2) {
                  if (is_usb) BT_init();
                    else BT_close();
                  is_usb = !is_usb, fn = false;
                }
              } else BootKeyboard.press(hidCode);
              break;
            case ps2::UsbKeyAction::KeyUp:
              if (hidCode == KEY_RIGHT_ALT) fn = false;
                else BootKeyboard.release(hidCode);
              break;
        }
    }
}

