#pragma once
#ifndef IRENDERERPROVIDER_H_
#define IRENDERERPROVIDER_H_

#include <vector>
#include "Camera.h"

class IRendererProvider {
public:
    virtual ~IRendererProvider() = default;
    virtual void RebuildGameField() = 0;
    virtual Camera& GetCamera() = 0;
    virtual void OnWindowResize(int newWidth, int newHeight) = 0;
    virtual void Draw() = 0;
};
#endif // IRENDERERPROVIDER_H_