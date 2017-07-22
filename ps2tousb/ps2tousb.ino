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
    BootKeyboard.begin();
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
                break;
            case ps2::UsbKeyAction::KeyUp:
                BootKeyboard.release(hidCode);
                break;
        }
    }
}

