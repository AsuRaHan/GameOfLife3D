#pragma once
#ifndef INPUTHANDLER_H_
#define INPUTHANDLER_H_

#include "InputEvent.h"
#include <unordered_map>
#include <functional>

class InputHandler {
public:
    InputHandler();
    ~InputHandler();

    void RegisterHandler(InputEvent::InputType type, std::function<void(const InputEvent&)> handler);
    void ProcessEvent(const InputEvent& event);

private:
    std::unordered_map<InputEvent::InputType, std::function<void(const InputEvent&)>> handlers;
};

#endif // INPUTHANDLER_H_