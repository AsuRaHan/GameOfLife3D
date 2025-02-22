#ifndef RENDERCONTROLLER_H
#define RENDERCONTROLLER_H

#include "IController.h"

class RenderController : public IController {
public:
    void Initialize() override;
    void Update(float deltaTime) override;
};

#endif // RENDERCONTROLLER_H