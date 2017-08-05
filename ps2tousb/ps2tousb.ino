#include "ps2_Keyboard.h"
#include "ps2_NullDiagnostics.h"
#include "ps2_UsbTranslator.h"
#include "HID-Project.h"
#include "rn42.h"
#include "KeyReport.h"

#define BT_OUT Serial1

static bool is_usb = true;
static ps2::NullDiagnostics diagnostics;
static ps2::UsbTranslator<ps2::NullDiagnostics> keyMapping(diagnostics);
static ps2::Keyboard<2,3,16, ps2::NullDiagnostics> ps2Keyboard(diagnostics);
static ps2::UsbKeyboardLeds ledValueLastSentToPs2 = ps2::UsbKeyboardLeds::none;
static KeyReport report;
static RN42<typeof(BT_OUT)> bt;

void setup() {
  analogWrite(4, 0);
  if (!is_usb) bt.init(BT_OUT);
  ps2Keyboard.begin();
  BootKeyboard.begin();
}

static bool fn = false;

void key_down(KeyboardKeycode hidCode){
  if (hidCode == KEY_RIGHT_ALT) fn = true;
    if (fn){
      if (hidCode == KEY_DOWN) 
        is_usb ? Consumer.write(MEDIA_PLAY_PAUSE) : bt.WriteConsumer(BT_OUT, RN_PLAY);
      if (hidCode == KEY_RIGHT) 
        is_usb ? Consumer.write(MEDIA_NEXT) : bt.WriteConsumer(BT_OUT, RN_NEXT);
      if (hidCode == KEY_LEFT) 
        is_usb ? Consumer.write(MEDIA_PREV) : bt.WriteConsumer(BT_OUT, RN_PREV);
      if (hidCode == KEY_UP) {
        is_usb ? Consumer.write(MEDIA_STOP) : bt.WriteConsumer(BT_OUT, RN_STOP); //BT Android problem
        fn = false;
      } 
      if (hidCode == KEY_F10) {
        is_usb ? Consumer.write(MEDIA_VOL_MUTE) : bt.WriteConsumer(BT_OUT, RN_MUTE);  
        fn = false;
      }
      if (hidCode == KEY_F11)
        is_usb ? Consumer.write(MEDIA_VOL_DOWN) : bt.WriteConsumer(BT_OUT, RN_VOL_DOWN);  
      if (hidCode == KEY_F12)
        is_usb ? Consumer.write(MEDIA_VOL_UP) : bt.WriteConsumer(BT_OUT, RN_VOL_UP);
      if (hidCode == KEY_F2) {
        is_usb ? bt.init(BT_OUT) : bt.close(BT_OUT);
        is_usb = !is_usb, fn = false;
      }
  } else if (is_usb) BootKeyboard.press(hidCode); else report.add(hidCode);
}

void key_up(KeyboardKeycode hidCode){
  if (hidCode == KEY_RIGHT_ALT) fn = false;
    else if (is_usb) BootKeyboard.release(hidCode); else report.remove(hidCode);
}

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
        key_down(hidCode);
        break;
      case ps2::UsbKeyAction::KeyUp:
        key_up(hidCode);    
        break;
    }
    if (!is_usb) bt.SendReport(BT_OUT, &report);
  }
}

