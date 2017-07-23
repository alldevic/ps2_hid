#include "ps2_Keyboard.h"
#include "ps2_NullDiagnostics.h"
#include "ps2_UsbTranslator.h"
#include "HID-Project.h"

static ps2::NullDiagnostics diagnostics;
static ps2::UsbTranslator<ps2::NullDiagnostics> keyMapping(diagnostics);
static ps2::Keyboard<2,3,16, ps2::NullDiagnostics> ps2Keyboard(diagnostics);
static ps2::UsbKeyboardLeds ledValueLastSentToPs2 = ps2::UsbKeyboardLeds::none;

void setup() {
    Serial1.begin(115200);
    ps2Keyboard.begin();
    BootKeyboard.begin();
}

void BTkeyboardPress(byte BP_KEY,byte BP_MOD){
  Serial1.write((byte)0xFD); //Start HID Report
  Serial1.write((byte)0x9); //Length byte
  Serial1.write((byte)0x1); //Descriptor byte
  Serial1.write(BP_MOD); //Modifier byte
  Serial1.write((byte)0x00); //-
  Serial1.write(BP_KEY); //Send KEY
  for(byte i = 0;i<5;i++) //Send five zero bytes
    Serial1.write((byte)0x00);
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
                BootKeyboard.press(hidCode);
                BTkeyboardPress(hidCode,(byte)0x00);     
                break;
            case ps2::UsbKeyAction::KeyUp:
                BootKeyboard.release(hidCode);
                BTkeyboardPress((byte)0x00,(byte)0x00);
                break;
        }
    }
}

