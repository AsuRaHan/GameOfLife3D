#ifndef APPLICATION_H
#define APPLICATION_H

#include <unordered_map>
#include <string>
#include "IController.h"

class Application {
private:
    std::unordered_map<std::string, IController*> controllers; // Реестр контроллеров

public:
    Application();
    ~Application();

    // Регистрация нового контроллера
    void RegisterController(const std::string& name, IController* controller);

    // Инициализация всех зарегистрированных контроллеров
    void InitializeControllers();

    // Обновление всех зарегистрированных контроллеров
    void UpdateControllers(float deltaTime);

    // Получение контроллера по имени
    IController* GetController(const std::string& name);
};

#endif // APPLICATION_H