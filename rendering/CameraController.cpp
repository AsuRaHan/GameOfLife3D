#include "CameraController.h"

CameraController::CameraController(Camera& cam) : camera(cam), moveSpeed(5.0f), rotationSpeed(0.1f)
, isMouseMiddleCaptured(false) 
, isMouseRighttCaptured(false)
{}

void CameraController::Update(float deltaTime) {
    // Можно использовать deltaTime для сглаживания движения камеры
    // Пример: если есть постоянное движение, можно применять его здесь
}

void CameraController::Rotate(float yaw, float pitch) {
    camera.Rotate(yaw * rotationSpeed, pitch * rotationSpeed);
}

void CameraController::Move(float dx, float dy, float dz) {
    float currentPosX, currentPosY, currentPosZ;
    camera.GetPosition(currentPosX, currentPosY, currentPosZ);

    // Минимальное расстояние, на которое камера может приблизиться к сетке
    float minZoomDistance = 1.0f; // или другая величина, которую вы считаете подходящей

    // Проверка на движение по оси Z
    if (dz < 0) { // движение вперед
        float newPosZ = currentPosZ + dz * moveSpeed;
        if (newPosZ < minZoomDistance) {
            camera.Move(dx * moveSpeed, dy * moveSpeed, minZoomDistance - currentPosZ);
        }
        else {
            camera.Move(dx * moveSpeed, dy * moveSpeed, dz * moveSpeed);
        }
    }
    else { // движение назад или нет движения по Z
        camera.Move(dx * moveSpeed, dy * moveSpeed, dz * moveSpeed);
    }
}

void CameraController::Zoom(float zoomAmount) {
    float currentPosX, currentPosY, currentPosZ;
    camera.GetPosition(currentPosX, currentPosY, currentPosZ);

    // Минимальное расстояние, на которое камера может приблизиться к сетке
    float minZoomDistance = 1.0f; // или другая величина, которую вы считаете подходящей

    // Приближение
    if (zoomAmount < 0) {
        float newPosZ = currentPosZ + zoomAmount * moveSpeed;
        if (newPosZ < minZoomDistance) {
            camera.Move(0.0f, 0.0f, minZoomDistance - currentPosZ); // Двигаем до минимального расстояния
        }
        else {
            camera.Move(0.0f, 0.0f, zoomAmount * moveSpeed);
        }
    }
    // Отдаление
    else {
        camera.Move(0.0f, 0.0f, zoomAmount * moveSpeed); // Нет ограничения на максимальную дистанцию
    }
}

void CameraController::SetTarget(float x, float y, float z) {
    // Устанавливаем точку, на которую смотрит камера
    camera.LookAt(x, y, z);
}

void CameraController::HandleInput(const InputEvent& event) {
    switch (event.type) {
    case InputEvent::InputType::MouseMove:
        // Вращение камеры на среднюю кнопку, перемещение на правую
        if (isMouseMiddleCaptured) {
            Rotate(event.deltaX, event.deltaY);
        }
        else if (isMouseRighttCaptured) {
            // Перемещаем камеру в горизонтальной плоскости
            Move(event.deltaX * 0.5f, event.deltaY * 0.5f, 0); // Умножаем на 0.5f для уменьшения скорости движения
        }
        break;
    case InputEvent::InputType::MouseWheel:
        Zoom(-event.wheelDelta); // Инвертируем, чтобы прокрутка вперед приближала
        break;
    case InputEvent::InputType::KeyDown:
        // Управление движением камеры клавиатурой остается без изменений
        switch (event.key) {
        case 'W': Move(0, 0, -1.1f); break; // Вперед
        case 'S': Move(0, 0, 1.1f); break; // Назад
        case 'A': Move(-1.1f, 0, 0); break; // Влево
        case 'D': Move(1.1f, 0, 0); break; // Вправо
        case 'Q': Move(0, 1.1f, 0); break; // Вверх
        case 'E': Move(0, -1.1f, 0); break; // Вниз
        default: break;
        }
        break;
    case InputEvent::InputType::MouseButtonDown:
        if (event.button == InputEvent::MouseButton::Middle) {
            isMouseMiddleCaptured = true;
        }
        else if (event.button == InputEvent::MouseButton::Right) {
            isMouseRighttCaptured = true;
        }
        break;
    case InputEvent::InputType::MouseButtonUp:
        if (event.button == InputEvent::MouseButton::Middle) {
            isMouseMiddleCaptured = false;
        }
        else if (event.button == InputEvent::MouseButton::Right) {
            isMouseRighttCaptured = false;
        }
        break;
    default:
        break;
    }
}