#include "ps2_Keyboard.h"
#include "ps2_NullDiagnostics.h"
#include "ps2_UsbTranslator.h"
#include "HID-Project.h"

static ps2::NullDiagnostics diagnostics;
static ps2::UsbTranslator<ps2::NullDiagnostics> keyMapping(diagnostics);
static ps2::Keyboard<2,3,16, ps2::NullDiagnostics> ps2Keyboard(diagnostics);
static ps2::UsbKeyboardLeds ledValueLastSentToPs2 = ps2::UsbKeyboardLeds::none;

void setup() {
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
              } else BootKeyboard.press(hidCode);
              break;
            case ps2::UsbKeyAction::KeyUp:
              if (hidCode == KEY_RIGHT_ALT) fn = false;
                else BootKeyboard.release(hidCode);
                
              break;
            
        }
    }
}

