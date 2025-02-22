#ifndef ICONTROLLER_H
#define ICONTROLLER_H

class IController {
public:
    virtual void Initialize() = 0;         // Инициализация контроллера
    virtual void Update(float deltaTime) = 0; // Обновление контроллера
    virtual ~IController() = default;      // Виртуальный деструктор для корректного удаления
};

#endif // ICONTROLLER_H