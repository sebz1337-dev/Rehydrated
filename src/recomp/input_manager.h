#pragma once

#include <list>
#include <memory>
#include <mutex>

#include <rex/ui/window.h>
#include <rex/ui/window_listener.h>

#include "game/input.h"

using InputQueue = std::list<BufferedInput>;

class InputManager : public rex::ui::WindowInputListener, public rex::ui::WindowListener
{
public:
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(InputManager&&) = delete;

    static InputManager& Instance()
    {
        static InputManager instance;
        return instance;
    }

    void Initialize(rex::ui::Window* window);

    // Last mouse button pressed by the host, used by the target-resolution hook to
    // distinguish move clicks (RMB) from attack clicks (LMB).
    static MouseButton LastMouseButton() { return m_lastMouseButton; }

    bool OnGuestInputUpdate();

    // WindowInputListener
    void OnKeyDown(rex::ui::KeyEvent& e) override;
    void OnKeyUp(rex::ui::KeyEvent& e) override;
    void OnMouseDown(rex::ui::MouseEvent& e) override;
    void OnMouseUp(rex::ui::MouseEvent& e) override;
    void OnMouseMove(rex::ui::MouseEvent& e) override;
    void OnMouseWheel(rex::ui::MouseEvent& e) override;

private:
    InputManager();
    ~InputManager() = default;

    void QueueInput(const BufferedInput& input);
    void SetMousePosition(BufferedInput& input, int32_t x, int32_t y);

    rex::ui::Window* m_window = nullptr;
    static MouseButton m_lastMouseButton;
    int m_buttonReleaseCountdown = 0;
    std::unique_ptr<InputQueue> m_currentInput;
    std::unique_ptr<InputQueue> m_pendingInput;
    std::mutex m_inputMutex;
};