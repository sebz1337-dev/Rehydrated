#include "recomp/input_manager.h"

#include <algorithm>

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ui/flags.h>
#include <rex/ui/virtual_key.h>

#include "core/memory.h"

REXCVAR_DEFINE_BOOL(enable_mnk_input, false, "Hydra/Gameplay", "(EXPERIMENTAL) Enables mouse and keyboard input")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

REXCVAR_DEFINE_BOOL(mnk_left_click_as_button, false, "Hydra/Gameplay",
                    "Also inject a synthetic A-button press on left mouse click")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

MouseButton InputManager::m_lastMouseButton = NO_MOUSE_BUTTON;

REXCVAR_DEFINE_BOOL(mnk_keyboard_as_controller, false, "Hydra/Gameplay",
                    "Translate keyboard keys into console controller actions (reuses SDK MNK bindings)")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

namespace
{

bool KeyToButton(rex::ui::VirtualKey vk, InputButton& button)
{
    using rex::ui::VirtualKey;
    switch (vk)
    {
        case VirtualKey::kOem1:   case VirtualKey::kSpace:     button = BUTTON_1; return true;  // A
        case VirtualKey::kOem7:   case VirtualKey::kBack:      button = BUTTON_2; return true;  // B
        case VirtualKey::kL:                                  button = BUTTON_3; return true;  // X
        case VirtualKey::kP:                                  button = BUTTON_4; return true;  // Y
        case VirtualKey::k1:      case VirtualKey::k3:
            button = vk == VirtualKey::k1 ? BUTTON_LEFT_SHOULDER : BUTTON_RIGHT_SHOULDER;
            return true;
        case VirtualKey::kQ:      case VirtualKey::kI:
            button = BUTTON_LEFT_TRIGGER; return true;
        case VirtualKey::kE:      case VirtualKey::kO:
            button = BUTTON_RIGHT_TRIGGER; return true;
        case VirtualKey::kReturn: case VirtualKey::kX:
            button = BUTTON_START; return true;
        case VirtualKey::kTab:    case VirtualKey::kZ:
            button = BUTTON_BACK; return true;
        default:
            return false;
    }
}

bool KeyToStick(rex::ui::VirtualKey vk, int32_t& x, int32_t& y)
{
    using rex::ui::VirtualKey;
    x = 0;
    y = 0;
    switch (vk)
    {
        case VirtualKey::kW: y = 1;  return true;
        case VirtualKey::kS: y = -1; return true;
        case VirtualKey::kA: x = -1; return true;
        case VirtualKey::kD: x = 1;  return true;
        default: return false;
    }
}

}  // namespace

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
    m_window = window;
    window->AddInputListener(this, 32);
    window->AddListener(this);
    REXLOG_INFO("Input initialized");
}

void InputManager::SetMousePosition(BufferedInput& input, int32_t x, int32_t y)
{
    // Mouse events arrive in window coordinates, but the guest resolves clicks
    // in its video mode coordinate space (1280x720 by default). Scale so the
    // cursor the user sees lines up with the target the game picks.
    if (m_window)
    {
        const uint32_t windowWidth = m_window->GetActualLogicalWidth();
        const uint32_t windowHeight = m_window->GetActualLogicalHeight();
        const int32_t guestWidth = std::max<int32_t>(1, REXCVAR_GET(video_mode_width));
        const int32_t guestHeight = std::max<int32_t>(1, REXCVAR_GET(video_mode_height));
        if (windowWidth && windowHeight)
        {
            x = static_cast<int32_t>(static_cast<int64_t>(x) * guestWidth / windowWidth);
            y = static_cast<int32_t>(static_cast<int64_t>(y) * guestHeight / windowHeight);
        }
    }

    input.tBufMouseData.p.x = x;
    input.tBufMouseData.p.y = y;
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
        // Force injection: the guest drops non-forced events while its internal
        // input-suppression counter is positive (cutscenes, menus, combat).
        InputBufferInput(currentInput.front(), 1);
        currentInput.pop_front();
    }

    if (m_buttonReleaseCountdown > 0 && --m_buttonReleaseCountdown == 0)
    {
        BufferedInput release = {};
        release.eInputMsg = INPUTMSG_CONTROLLERBUTTONUP;
        release.tBufControllerData.nControllerIndex = 0;
        release.tBufControllerData.eInputButton = BUTTON_1;
        QueueInput(release);
    }

    return true;
}

void InputManager::OnMouseDown(rex::ui::MouseEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    BufferedInput tInput = {};
    const bool leftAsButton = REXCVAR_GET(mnk_left_click_as_button) &&
                              e.button() == rex::ui::MouseEvent::Button::kLeft;

    switch (e.button())
    {
        case rex::ui::MouseEvent::Button::kLeft:
            m_lastMouseButton = LEFT_MOUSE_BUTTON;
            if (!leftAsButton)
            {
                tInput.eInputMsg = INPUTMSG_LEFTMOUSEDOWN;
                tInput.tBufMouseData.eMouseButton = LEFT_MOUSE_BUTTON;
            }
            break;

        case rex::ui::MouseEvent::Button::kRight:
            tInput.eInputMsg = INPUTMSG_RIGHTMOUSEDOWN;
            tInput.tBufMouseData.eMouseButton = RIGHT_MOUSE_BUTTON;
            m_lastMouseButton = RIGHT_MOUSE_BUTTON;
            // Release a pending synthetic A press immediately: a right-click landing
            // while A is still held becomes an attack instead of a move.
            if (m_buttonReleaseCountdown > 0)
            {
                BufferedInput release = {};
                release.eInputMsg = INPUTMSG_CONTROLLERBUTTONUP;
                release.tBufControllerData.nControllerIndex = 0;
                release.tBufControllerData.eInputButton = BUTTON_1;
                SetMousePosition(release, e.x(), e.y());
                QueueInput(release);
                m_buttonReleaseCountdown = 0;
            }
            break;

        case rex::ui::MouseEvent::Button::kMiddle:
            tInput.eInputMsg = INPUTMSG_MIDDLEMOUSEDOWN;
            tInput.tBufMouseData.eMouseButton = MIDDLE_MOUSE_BUTTON;
            m_lastMouseButton = MIDDLE_MOUSE_BUTTON;
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
    
    if (!leftAsButton)
    {
        SetMousePosition(tInput, e.x(), e.y());
        QueueInput(tInput);
    }

    if (e.button() == rex::ui::MouseEvent::Button::kLeft)
    {
        if (!leftAsButton)
        {
            // D3's attack/interact path reads the "still down" state rather than the
            // initial press, so a down/up pair inside one guest tick does nothing.
            tInput.eInputMsg = INPUTMSG_LEFTMOUSESTILLDOWN;
            QueueInput(tInput);
        }
        else
        {
            // In A-button mode inject ONLY the controller press for this click.
            // Injecting native LEFTMOUSEDOWN as well leaves conflicting attack state
            // that a later right-click can inherit.
            BufferedInput button = {};
            button.eInputMsg = INPUTMSG_CONTROLLERBUTTONDOWN;
            button.tBufControllerData.nControllerIndex = 0;
            button.tBufControllerData.eInputButton = BUTTON_1;
            SetMousePosition(button, e.x(), e.y());
            QueueInput(button);
            m_buttonReleaseCountdown = 1;
        }
    }
}

void InputManager::OnMouseUp(rex::ui::MouseEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    BufferedInput tInput = {};
    const bool leftAsButton = REXCVAR_GET(mnk_left_click_as_button) &&
                              e.button() == rex::ui::MouseEvent::Button::kLeft;

    switch (e.button())
    {
        case rex::ui::MouseEvent::Button::kLeft:
            if (!leftAsButton)
            {
                tInput.eInputMsg = INPUTMSG_LEFTMOUSEUP;
                tInput.tBufMouseData.eMouseButton = LEFT_MOUSE_BUTTON;
            }
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

    if (leftAsButton)
        return;

    SetMousePosition(tInput, e.x(), e.y());

    QueueInput(tInput);
}

void InputManager::OnMouseMove(rex::ui::MouseEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    BufferedInput tInput = {};
    tInput.eInputMsg = INPUTMSG_MOUSEMOVE;
    tInput.tBufMouseData.eMouseButton = NO_MOUSE_BUTTON;
    SetMousePosition(tInput, e.x(), e.y());

    QueueInput(tInput);
}

void InputManager::OnMouseWheel(rex::ui::MouseEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input) || e.scroll_y() == 0)
        return;

    BufferedInput tInput = {};
    tInput.eInputMsg = INPUTMSG_MOUSEWHEEL;
    tInput.tBufMouseData.eMouseButton = NO_MOUSE_BUTTON;
    SetMousePosition(tInput, e.x(), e.y());
    tInput.tBufMouseData.nDeltaMouseWheel = e.scroll_y() / rex::ui::MouseEvent::kScrollPerDetent;

    QueueInput(tInput);
}

void InputManager::OnKeyDown(rex::ui::KeyEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    if (REXCVAR_GET(mnk_keyboard_as_controller))
    {
        const auto vk = e.virtual_key();
        InputButton button = BUTTON_INVALID;
        int32_t sx = 0, sy = 0;
        if (KeyToStick(vk, sx, sy))
        {
            BufferedInput input = {};
            input.eInputMsg = INPUTMSG_LEFTSTICK;
            input.tBufControllerData.nControllerIndex = 0;
            input.tBufControllerData.vLeftStick.x = static_cast<float>(sx);
            input.tBufControllerData.vLeftStick.y = static_cast<float>(sy);
            QueueInput(input);
            return;
        }
        if (KeyToButton(vk, button))
        {
            BufferedInput input = {};
            input.eInputMsg = INPUTMSG_CONTROLLERBUTTONDOWN;
            input.tBufControllerData.nControllerIndex = 0;
            input.tBufControllerData.eInputButton = button;
            QueueInput(input);
            return;
        }
    }

    const Key key = GameKeyFromVirtualKey(e.virtual_key());
    if (key == KEY_NONE)
        return;

    BufferedInput tInput = {};
    tInput.eInputMsg = INPUTMSG_KEYDOWN;
    tInput.tBufKeyData.wPlatformKey = static_cast<u32>(key) << 16;
    tInput.tBufKeyData.eKey = key;

    QueueInput(tInput);
}

void InputManager::OnKeyUp(rex::ui::KeyEvent& e)
{
    if (!REXCVAR_GET(enable_mnk_input))
        return;

    if (REXCVAR_GET(mnk_keyboard_as_controller))
    {
        const auto vk = e.virtual_key();
        InputButton button = BUTTON_INVALID;
        int32_t sx = 0, sy = 0;
        if (KeyToStick(vk, sx, sy))
        {
            BufferedInput input = {};
            input.eInputMsg = INPUTMSG_LEFTSTICK;
            input.tBufControllerData.nControllerIndex = 0;
            input.tBufControllerData.vLeftStick.x = 0.0f;
            input.tBufControllerData.vLeftStick.y = 0.0f;
            QueueInput(input);
            return;
        }
        if (KeyToButton(vk, button))
        {
            BufferedInput input = {};
            input.eInputMsg = INPUTMSG_CONTROLLERBUTTONUP;
            input.tBufControllerData.nControllerIndex = 0;
            input.tBufControllerData.eInputButton = button;
            QueueInput(input);
            return;
        }
    }

    const Key key = GameKeyFromVirtualKey(e.virtual_key());
    if (key == KEY_NONE)
        return;

    BufferedInput tInput = {};
    tInput.eInputMsg = INPUTMSG_KEYUP;
    tInput.tBufKeyData.wPlatformKey = static_cast<u32>(key) << 16;
    tInput.tBufKeyData.eKey = key;

    QueueInput(tInput);
}

void InputManager::QueueInput(const BufferedInput& input)
{
    std::unique_lock<std::mutex> lock(m_inputMutex);
    m_pendingInput->push_back(input);
}
