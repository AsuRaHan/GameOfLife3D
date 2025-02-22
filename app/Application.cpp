#include "Application.h"

Application::Application() {}

Application::~Application() {
    // Освобождаем память, выделенную под контроллеры
    for (auto& pair : controllers) {
        delete pair.second;
    }
    controllers.clear();
}

void Application::RegisterController(const std::string& name, IController* controller) {
    controllers[name] = controller; // Добавляем контроллер в реестр
}

void Application::InitializeControllers() {
    // Проходим по всем контроллерам и вызываем их Initialize
    for (auto& pair : controllers) {
        pair.second->Initialize();
    }
}

void Application::UpdateControllers(float deltaTime) {
    // Проходим по всем контроллерам и вызываем их Update
    for (auto& pair : controllers) {
        pair.second->Update(deltaTime);
    }
}

IController* Application::GetController(const std::string& name) {
    // Ищем контроллер по имени и возвращаем его или nullptr, если не найден
    auto it = controllers.find(name);
    if (it != controllers.end()) {
        return it->second;
    }
    return nullptr;
}