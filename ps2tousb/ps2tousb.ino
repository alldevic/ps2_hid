#include "ps2_Keyboard.h"
#include "ps2_NullDiagnostics.h"
#include "ps2_UsbTranslator.h"
#include "HID-Project.h"

// RN-42 consumer codes
#define BT_RELEASE 0x0        // Release consumer key
#define BT_HOME 0x1           // AC Home
#define BT_EMAIL 0x2          // AC Email Reader
#define BT_SEARCH 0x4         // AC Search
#define BT_KEYBLAY 0x8        // AL Keyboard Layout (Virtual Apple Keyboard Toggle)
#define BT_VOL_UP 0x10        // Volume Up
#define BT_VOL_DOWN 0x20      // Volume Down
#define BT_MUTE 0x40          // Mute
#define BT_PLAY 0x80          // Play/Pause
#define BT_NEXT 0x100         // Scan Next Track
#define BT_PREV 0x200         // Scan Previous Track
#define BT_STOP 0x400         // Stop
#define BT_EJECT 0x800        // Eject
#define BT_FORWARD 0x1000     // Fast Forward
#define BT_REWIND 0x2000      // Rewind
#define BT_STOP_EJECT 0x4000  // Stop/Eject
#define BT_BROWSER 0x8000     // AL Internet Browse

static bool is_usb = true;
static ps2::NullDiagnostics diagnostics;
static ps2::UsbTranslator<ps2::NullDiagnostics> keyMapping(diagnostics);
static ps2::Keyboard<2,3,16, ps2::NullDiagnostics> ps2Keyboard(diagnostics);
static ps2::UsbKeyboardLeds ledValueLastSentToPs2 = ps2::UsbKeyboardLeds::none;

void BT_init(){
  analogWrite(4, 255);
  if (Serial1.available()) Serial1.end();
  delay(100);
  Serial1.begin(115200);
  delay(320);                     // IMPORTANT DELAY! (Minimum ~276ms)
  Serial1.print("$$$");         // Enter command mode
  delay(15);
  Serial1.print("CFR\n");                 
  delay(100);
}

void BT_close(){
  digitalWrite(4, LOW);
  if (Serial1.available()) Serial1.end();
}

void BT_WriteConsumer(byte consumer){
  Serial1.write((byte)0xFD); //Start HID Report 
  Serial1.write((byte)0x3); //Length byte 
  Serial1.write((byte)0x3); //Descriptor byte 
  Serial1.write(consumer); 
  Serial1.write((byte)0x0);
  delay(15);
  if (consumer != BT_RELEASE)
    BT_WriteConsumer(BT_RELEASE);
  return;
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
  BootKeyboard.begin();
}

void pass(){
  return;
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
                if (hidCode == KEY_DOWN) 
                  is_usb ? Consumer.write(MEDIA_PLAY_PAUSE) : BT_WriteConsumer(BT_PLAY);
                if (hidCode == KEY_RIGHT) 
                  is_usb ? Consumer.write(MEDIA_NEXT) : BT_WriteConsumer(BT_NEXT);
                if (hidCode == KEY_LEFT) 
                  is_usb ? Consumer.write(MEDIA_PREV) : BT_WriteConsumer(BT_PREV);  
                if (hidCode == KEY_UP) {
                  is_usb ? Consumer.write(MEDIA_STOP) : BT_WriteConsumer(BT_STOP);;
                  fn = false;
                } 
                if (hidCode == KEY_F10) {
                  is_usb ? Consumer.write(MEDIA_VOL_MUTE) : BT_WriteConsumer(BT_MUTE);  
                   fn = false;
                }
                if (hidCode == KEY_F11)
                  is_usb ? Consumer.write(MEDIA_VOL_DOWN) : BT_WriteConsumer(BT_VOL_DOWN);  
                if (hidCode == KEY_F12)
                  is_usb ? Consumer.write(MEDIA_VOL_UP) : BT_WriteConsumer(BT_VOL_UP);
                if (hidCode == KEY_F2) {
                  is_usb ? BT_init() : BT_close();
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

