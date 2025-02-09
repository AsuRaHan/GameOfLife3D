#include "InputHandler.h"

InputHandler::InputHandler() {}

InputHandler::~InputHandler() {}

void InputHandler::RegisterHandler(InputEvent::InputType type, std::function<void(const InputEvent&)> handler) {
    handlers[type] = handler;
}

void InputHandler::ProcessEvent(const InputEvent& event) {
    auto it = handlers.find(event.type);
    if (it != handlers.end()) {
        it->second(event);  // Вызываем соответствующий обработчик
    }
}