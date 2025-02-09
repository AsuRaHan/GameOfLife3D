#pragma once
#ifndef CAMERACONTROLLER_H_
#define CAMERACONTROLLER_H_

#include "../system/InputEvent.h"
#include "Camera.h"
#include <cmath>

// Предполагается, что у вас есть структура для событий ввода


class CameraController {
public:
    CameraController(Camera& cam);
    void Update(float deltaTime);
    void Rotate(float yaw, float pitch);
    void Move(float dx, float dy, float dz);
    void Zoom(float zoomAmount);
    void SetTarget(float x, float y, float z);
    void HandleInput(const InputEvent& event);

private:
    Camera& camera;
    float moveSpeed;
    float rotationSpeed;
    bool isMouseMiddleCaptured;
    bool isMouseRighttCaptured;
};

#endif // CAMERACONTROLLER_H_