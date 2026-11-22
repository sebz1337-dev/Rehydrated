#include "recomp/input_manager.h"

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ui/virtual_key.h>

#include "core/memory.h"

REXCVAR_DEFINE_BOOL(enable_mnk_input, false, "Hydra/Gameplay", "(EXPERIMENTAL) Enables mouse and keyboard input")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

namespace
{

REX_IMPORT(__imp__InputBufferInput, Guest_InputBufferInput, void(u32, u32));

void InputBufferInput(const BufferedInput& tInput, int bForce)
{
    StackFrame frame;
    u32 ptInput = frame.Push(tInput);

    Guest_InputBufferInput(ptInput, bForce);
}

// HydraHook_InputPlatformKeyConvert is a no-op, so wPlatformKey reaches the
// guest as eKey directly. Map host virtual keys to the game's Key enum.
// NOTE: inferred from the hook and the game enum; needs in-game verification.
Key GameKeyFromVirtualKey(rex::ui::VirtualKey vk)
{
    using rex::ui::VirtualKey;
    switch (vk)
    {
        case VirtualKey::kEscape:  return KEY_ESCAPE;
        case VirtualKey::kBack:    return KEY_BACK;
        case VirtualKey::kTab:     return KEY_TAB;
        case VirtualKey::kReturn:  return KEY_RETURN;
        case VirtualKey::kShift:
        case VirtualKey::kLShift:  return KEY_LSHIFT;
        case VirtualKey::kRShift:  return KEY_RSHIFT;
        case VirtualKey::kControl:
        case VirtualKey::kLControl: return KEY_LCONTROL;
        case VirtualKey::kRControl: return KEY_RCONTROL;
        case VirtualKey::kMenu:
        case VirtualKey::kLMenu:   return KEY_LMENU;
        case VirtualKey::kRMenu:   return KEY_RMENU;
        case VirtualKey::kPause:   return KEY_PAUSE;
        case VirtualKey::kCapital: return KEY_CAPITAL;
        case VirtualKey::kSpace:   return KEY_SPACE;
        case VirtualKey::kPrior:   return KEY_PRIOR;
        case VirtualKey::kNext:    return KEY_NEXT;
        case VirtualKey::kEnd:     return KEY_END;
        case VirtualKey::kHome:    return KEY_HOME;
        case VirtualKey::kLeft:    return KEY_LEFT;
        case VirtualKey::kUp:      return KEY_UP;
        case VirtualKey::kRight:   return KEY_RIGHT;
        case VirtualKey::kDown:    return KEY_DOWN;
        case VirtualKey::kInsert:  return KEY_INSERT;
        case VirtualKey::kDelete:  return KEY_DELETE;
        case VirtualKey::kLWin:    return KEY_LWIN;
        case VirtualKey::kRWin:    return KEY_RWIN;
        case VirtualKey::kApps:    return KEY_APPS;
        case VirtualKey::k0: return KEY_0;
        case VirtualKey::k1: return KEY_1;
        case VirtualKey::k2: return KEY_2;
        case VirtualKey::k3: return KEY_3;
        case VirtualKey::k4: return KEY_4;
        case VirtualKey::k5: return KEY_5;
        case VirtualKey::k6: return KEY_6;
        case VirtualKey::k7: return KEY_7;
        case VirtualKey::k8: return KEY_8;
        case VirtualKey::k9: return KEY_9;
        case VirtualKey::kA: return KEY_A;
        case VirtualKey::kB: return KEY_B;
        case VirtualKey::kC: return KEY_C;
        case VirtualKey::kD: return KEY_D;
        case VirtualKey::kE: return KEY_E;
        case VirtualKey::kF: return KEY_F;
        case VirtualKey::kG: return KEY_G;
        case VirtualKey::kH: return KEY_H;
        case VirtualKey::kI: return KEY_I;
        case VirtualKey::kJ: return KEY_J;
        case VirtualKey::kK: return KEY_K;
        case VirtualKey::kL: return KEY_L;
        case VirtualKey::kM: return KEY_M;
        case VirtualKey::kN: return KEY_N;
        case VirtualKey::kO: return KEY_O;
        case VirtualKey::kP: return KEY_P;
        case VirtualKey::kQ: return KEY_Q;
        case VirtualKey::kR: return KEY_R;
        case VirtualKey::kS: return KEY_S;
        case VirtualKey::kT: return KEY_T;
        case VirtualKey::kU: return KEY_U;
        case VirtualKey::kV: return KEY_V;
        case VirtualKey::kW: return KEY_W;
        case VirtualKey::kX: return KEY_X;
        case VirtualKey::kY: return KEY_Y;
        case VirtualKey::kZ: return KEY_Z;
        case VirtualKey::kF1:  return KEY_F1;
        case VirtualKey::kF2:  return KEY_F2;
        case VirtualKey::kF3:  return KEY_F3;
        case VirtualKey::kF4:  return KEY_F4;
        case VirtualKey::kF5:  return KEY_F5;
        case VirtualKey::kF6:  return KEY_F6;
        case VirtualKey::kF7:  return KEY_F7;
        case VirtualKey::kF8:  return KEY_F8;
        case VirtualKey::kF9:  return KEY_F9;
        case VirtualKey::kF10: return KEY_F10;
        case VirtualKey::kF11: return KEY_F11;
        case VirtualKey::kF12: return KEY_F12;
        case VirtualKey::kNumLock:    return KEY_NUMLOCK;
        case VirtualKey::kScroll:     return KEY_SCROLL;
        case VirtualKey::kNumpad0: return KEY_NUMPAD0;
        case VirtualKey::kNumpad1: return KEY_NUMPAD1;
        case VirtualKey::kNumpad2: return KEY_NUMPAD2;
        case VirtualKey::kNumpad3: return KEY_NUMPAD3;
        case VirtualKey::kNumpad4: return KEY_NUMPAD4;
        case VirtualKey::kNumpad5: return KEY_NUMPAD5;
        case VirtualKey::kNumpad6: return KEY_NUMPAD6;
        case VirtualKey::kNumpad7: return KEY_NUMPAD7;
        case VirtualKey::kNumpad8: return KEY_NUMPAD8;
        case VirtualKey::kNumpad9: return KEY_NUMPAD9;
        case VirtualKey::kMultiply: return KEY_MULTIPLY;
        case VirtualKey::kAdd:      return KEY_ADD;
        case VirtualKey::kSubtract: return KEY_SUBTRACT;
        case VirtualKey::kDecimal:  return KEY_DECIMAL;
        case VirtualKey::kDivide:   return KEY_DIVIDE;
        case VirtualKey::kOem1:     return KEY_SEMICOLON;
        case VirtualKey::kOemPlus:  return KEY_EQUALS;
        case VirtualKey::kOemComma: return KEY_COMMA;
        case VirtualKey::kOemMinus: return KEY_MINUS;
        case VirtualKey::kOemPeriod: return KEY_PERIOD;
        case VirtualKey::kOem2:     return KEY_SLASH;
        case VirtualKey::kOem3:     return KEY_GRAVE;
        case VirtualKey::kOem4:     return KEY_LBRACKET;
        case VirtualKey::kOem5:     return KEY_BACKSLASH;
        case VirtualKey::kOem6:     return KEY_RBRACKET;
        case VirtualKey::kOem7:     return KEY_APOSTROPHE;
        default: return KEY_NONE;
    }
}

}

InputManager::InputManager() :
    m_currentInput(std::make_unique<InputQueue>()),
    m_pendingInput(std::make_unique<InputQueue>())
{
}

void InputManager::Initialize(rex::ui::Window* window)
{
    window->AddInputListener(this, 32);
    window->AddListener(this);
    REXLOG_INFO("Input initialized");
}

bool InputManager::OnGuestInputUpdate()
{
    {
        std::unique_lock<std::mutex> lock(m_inputMutex);
        std::swap(m_currentInput, m_pendingInput);
    }

    InputQueue& currentInput = *m_currentInput;

    if (!REXCVAR_GET(enable_mnk_input))
    {
        currentInput.clear();
        return false;
    }

    while (!currentInput.empty())
    {
        InputBufferInput(currentInput.front(), 0);
        currentInput.pop_front();
    }

    return true;
}

void InputManager::OnMouseDown(rex::ui::MouseEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    BufferedInput tInput = {};

    switch (e.button())
    {
        case rex::ui::MouseEvent::Button::kLeft:
            tInput.eInputMsg = INPUTMSG_LEFTMOUSEDOWN;
            tInput.tBufMouseData.eMouseButton = LEFT_MOUSE_BUTTON;
            break;

        case rex::ui::MouseEvent::Button::kRight:
            tInput.eInputMsg = INPUTMSG_RIGHTMOUSEDOWN;
            tInput.tBufMouseData.eMouseButton = RIGHT_MOUSE_BUTTON;
            break;

        case rex::ui::MouseEvent::Button::kMiddle:
            tInput.eInputMsg = INPUTMSG_MIDDLEMOUSEDOWN;
            tInput.tBufMouseData.eMouseButton = MIDDLE_MOUSE_BUTTON;
            break;

        case rex::ui::MouseEvent::Button::kX1:
            tInput.eInputMsg = INPUTMSG_KEYDOWN;
            tInput.tBufKeyData.wPlatformKey = KEY_MOUSE4;
            break;

        case rex::ui::MouseEvent::Button::kX2:
            tInput.eInputMsg = INPUTMSG_KEYDOWN;
            tInput.tBufKeyData.wPlatformKey = KEY_MOUSE5;
            break;

        default:
            return;
    }
    
    tInput.tBufMouseData.p.x = e.x();
    tInput.tBufMouseData.p.y = e.y();

    QueueInput(tInput);
}

void InputManager::OnMouseUp(rex::ui::MouseEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    BufferedInput tInput = {};

    switch (e.button())
    {
        case rex::ui::MouseEvent::Button::kLeft:
            tInput.eInputMsg = INPUTMSG_LEFTMOUSEUP;
            tInput.tBufMouseData.eMouseButton = LEFT_MOUSE_BUTTON;
            break;

        case rex::ui::MouseEvent::Button::kRight:
            tInput.eInputMsg = INPUTMSG_RIGHTMOUSEUP;
            tInput.tBufMouseData.eMouseButton = RIGHT_MOUSE_BUTTON;
            break;

        case rex::ui::MouseEvent::Button::kMiddle:
            tInput.eInputMsg = INPUTMSG_MIDDLEMOUSEUP;
            tInput.tBufMouseData.eMouseButton = MIDDLE_MOUSE_BUTTON;
            break;

        case rex::ui::MouseEvent::Button::kX1:
            tInput.eInputMsg = INPUTMSG_KEYUP;
            tInput.tBufKeyData.wPlatformKey = KEY_MOUSE4;
            break;

        case rex::ui::MouseEvent::Button::kX2:
            tInput.eInputMsg = INPUTMSG_KEYUP;
            tInput.tBufKeyData.wPlatformKey = KEY_MOUSE5;
            break;

        default:
            return;
    }

    tInput.tBufMouseData.p.x = e.x();
    tInput.tBufMouseData.p.y = e.y();

    QueueInput(tInput);
}

void InputManager::OnMouseMove(rex::ui::MouseEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    BufferedInput tInput = {};
    tInput.eInputMsg = INPUTMSG_MOUSEMOVE;
    tInput.tBufMouseData.eMouseButton = NO_MOUSE_BUTTON;
    tInput.tBufMouseData.p.x = e.x();
    tInput.tBufMouseData.p.y = e.y();

    QueueInput(tInput);
}

void InputManager::OnMouseWheel(rex::ui::MouseEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input) || e.scroll_y() == 0)
        return;

    BufferedInput tInput = {};
    tInput.eInputMsg = INPUTMSG_MOUSEWHEEL;
    tInput.tBufMouseData.eMouseButton = NO_MOUSE_BUTTON;
    tInput.tBufMouseData.p.x = e.x();
    tInput.tBufMouseData.p.y = e.y();
    tInput.tBufMouseData.nDeltaMouseWheel = e.scroll_y() / rex::ui::MouseEvent::kScrollPerDetent;

    QueueInput(tInput);
}

void InputManager::OnKeyDown(rex::ui::KeyEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    const Key key = GameKeyFromVirtualKey(e.virtual_key());
    if (key == KEY_NONE)
        return;

    BufferedInput tInput = {};
    tInput.eInputMsg = INPUTMSG_KEYDOWN;
    tInput.tBufKeyData.wPlatformKey = key;
    tInput.tBufKeyData.eKey = key;

    QueueInput(tInput);
}

void InputManager::OnKeyUp(rex::ui::KeyEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    const Key key = GameKeyFromVirtualKey(e.virtual_key());
    if (key == KEY_NONE)
        return;

    BufferedInput tInput = {};
    tInput.eInputMsg = INPUTMSG_KEYUP;
    tInput.tBufKeyData.wPlatformKey = key;
    tInput.tBufKeyData.eKey = key;

    QueueInput(tInput);
}

void InputManager::QueueInput(const BufferedInput& input)
{
    std::unique_lock<std::mutex> lock(m_inputMutex);
    m_pendingInput->push_back(input);
}
