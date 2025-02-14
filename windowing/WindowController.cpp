#include "WindowController.h"

#include <iostream>

WindowController::WindowController(IRendererProvider* renderer, GameController* gameController)
    : pRenderer(renderer), pGameController(gameController),
    mouseCaptured(false), lastMouseX(0), lastMouseY(0), isMiddleButtonDown(false),
    gridPicker(renderer->GetCamera())
    , inputHandler(),
    windowWidth(1024), windowHeight(768),
    pCameraController(renderer->GetCamera())
{
    // Регистрируем обработчики ввода для камеры
    inputHandler.RegisterHandler(InputEvent::InputType::MouseMove, [this](const InputEvent& event) {
        pCameraController.HandleInput(event);
        });

    inputHandler.RegisterHandler(InputEvent::InputType::MouseWheel, [this](const InputEvent& event) {
        pCameraController.HandleInput(event);
        });

    inputHandler.RegisterHandler(InputEvent::InputType::KeyDown, [this](const InputEvent& event) {
        pCameraController.HandleInput(event);
        });

    inputHandler.RegisterHandler(InputEvent::InputType::MouseButtonDown, [this](const InputEvent& event) {
        if (event.button == InputEvent::MouseButton::Right) {
            mouseCaptured = true;
            pCameraController.HandleInput(event);
        }
        });

    inputHandler.RegisterHandler(InputEvent::InputType::MouseButtonUp, [this](const InputEvent& event) {
        if (event.button == InputEvent::MouseButton::Right) {
            mouseCaptured = false;
            pCameraController.HandleInput(event);
        }
        });
    ResetCamera();
}

WindowController::~WindowController() {

}


void WindowController::Resize(int width, int height) {
    if (pRenderer) {
        pRenderer->OnWindowResize(width, height);
    }
}

// Изменяем HandleEvent для использования InputHandler
void WindowController::HandleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
    InputEvent event;
    event.deltaX = 0;
    event.deltaY = 0;
    event.wheelDelta = 0;
    event.key = 0;

    switch (message) {
    case WM_PAINT:
        if (pRenderer)pRenderer->Draw();
        break;
    case WM_SIZE:
        windowWidth = LOWORD(lParam);
        windowHeight = HIWORD(lParam);
        Resize(windowWidth, windowHeight); // Обработка изменения размеров окна
        break;
    case WM_MOUSEMOVE:
        event.type = InputEvent::InputType::MouseMove;
        event.deltaX = static_cast<float>(LOWORD(lParam) - lastMouseX) / 10.0f;
        event.deltaY = static_cast<float>(HIWORD(lParam) - lastMouseY) / 10.0f;
        lastMouseX = LOWORD(lParam);
        lastMouseY = HIWORD(lParam);
        inputHandler.ProcessEvent(event);
        break;
    case WM_MOUSEWHEEL:
        event.type = InputEvent::InputType::MouseWheel;
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            event.wheelDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA) * 10; // Увеличиваем скорость зума в 10 раз
        }
        else {
            event.wheelDelta = static_cast<float>GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        }
        inputHandler.ProcessEvent(event);
        break;
    case WM_LBUTTONDOWN: // Левый клик мыши
        if (pRenderer) {
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
        }
        HandleMouseClick(LOWORD(lParam), HIWORD(lParam));



        break;
    case WM_MBUTTONDOWN: // Нажатие средней кнопки мыши
        PlacePattern(LOWORD(lParam), HIWORD(lParam));
        break;


    case WM_KEYDOWN:
        event.type = InputEvent::InputType::KeyDown;
        event.key = static_cast<int>(wParam);
        inputHandler.ProcessEvent(event);
        // Здесь может быть дополнительная обработка клавиш, не связанных с камерой
        switch (wParam) {

        case VK_SPACE: // Пробел для запуска/остановки симуляции
            if (pGameController->isSimulationRunning()) {
                pGameController->stopSimulation();
            }
            else {
                pGameController->startSimulation();
            }
            break;
        case VK_RIGHT: // Стрелка вправо для следующего поколения
            if (!pGameController->isSimulationRunning()) {
                pGameController->stepSimulation(); // Переход к следующему поколению
            }
            break;
        case 'R':
            if (!pGameController->isSimulationRunning()) {
                pGameController->randomizeGrid(0.1f); // Случайное заполнение поля
            }
            break;
        case 'T':
            ResetCamera();
            break;
        case 'C':
            if (!pGameController->isSimulationRunning()) {
                pGameController->clearGrid(); // Очищение поля. убить всех
            }
            break;
        case 'Y':
            if (!pGameController->isSimulationRunning()) {
                pGameController->setWoldToroidal(!pGameController->getWoldToroidal()); // сделать мир безграничным или на оборот ограничеть его
            }
            break;
        case 'I':
            if (!pGameController->isSimulationRunning()) {
                pGameController->randomizeGrid(); // засеять поле рандомными фигурами
            }
            break;
        case 'G':
            pGameController->setShowGrid(!pGameController->getShowGrid());
            break;

        }
        break;
    //case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        event.type = message == WM_LBUTTONDOWN ? InputEvent::InputType::MouseButtonDown : InputEvent::InputType::MouseButtonDown;
        event.button = message == WM_LBUTTONDOWN ? InputEvent::MouseButton::Left : InputEvent::MouseButton::Right;
        inputHandler.ProcessEvent(event);
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        event.type = message == WM_LBUTTONUP ? InputEvent::InputType::MouseButtonUp : InputEvent::InputType::MouseButtonUp;
        event.button = message == WM_LBUTTONUP ? InputEvent::MouseButton::Left : InputEvent::MouseButton::Right;
        inputHandler.ProcessEvent(event);
        break;
        // Добавьте обработку других сообщений по мере необходимости
    default:
        break;
    }
}

void WindowController::HandleMouseClick(int screenX, int screenY) {
    float worldX, worldY;
    gridPicker.ScreenToWorld(static_cast<float>(screenX), static_cast<float>(screenY), static_cast<float>(windowWidth), static_cast<float>(windowHeight), worldX, worldY);

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    int x = static_cast<int>(worldX / cellSize);
    int y = static_cast<int>(worldY / cellSize);

    if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
        pGameController->toggleCellState(x, y);
    }
}

void WindowController::RotateCamera(float yaw, float pitch) {
    if (pRenderer) {
        Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
        camera.Rotate(yaw, pitch);
    }
}

void WindowController::MoveCamera(float dx, float dy) {
    if (pRenderer) {
        Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());

        // Получаем текущее направление и позицию камеры
        float dirX, dirY, dirZ;
        camera.GetDirection(dirX, dirY, dirZ);
        float currentPosX, currentPosY, currentPosZ;
        camera.GetPosition(currentPosX, currentPosY, currentPosZ);

        // Вычисляем движение камеры
        float moveX = dirX * dy;
        float moveZ = dirZ * dy;

        // Двигаем камеру влево/вправо перпендикулярно направлению взгляда
        float rightX = dirZ;
        float rightZ = -dirX;
        moveX += rightX * dx;
        moveZ += rightZ * dx;

        // Вычисляем новую позицию по Z
        float newPosZ = currentPosZ + moveZ;

        // Ограничиваем движение, чтобы камера не уходила за пределы z = 0
        if (newPosZ < -10) {
            moveZ = -currentPosZ;  // Двигаем ровно до нуля
        }

        // Применяем движение
        camera.Move(moveX, 0.0f, moveZ);
    }
}

void WindowController::PlacePattern(int screenX, int screenY) {
    float worldX, worldY;
    gridPicker.ScreenToWorld(static_cast<float>(screenX), static_cast<float>(screenY), static_cast<float>(windowWidth), static_cast<float>(windowHeight), worldX, worldY);

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // Преобразуем мировые координаты в координаты сетки
    int x = static_cast<int>(worldX / cellSize);
    int y = static_cast<int>(worldY / cellSize);

    // При размещении шаблона, убедитесь, что он не выходит за границы сетки
    if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
        pGameController->PlacePattern(x, y);
    }
}

void WindowController::ResetCamera() {
    if (pRenderer && pGameController) {
        Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
        float width = pGameController->getGridWidth() * pGameController->getCellSize();
        float height = pGameController->getGridHeight() * pGameController->getCellSize();
        // Определяем необходимый zoom, чтобы все поле было видно
        float windowAspectRatio = static_cast<float>(windowWidth) / windowHeight;
        float fieldAspectRatio = width / height;
        float tanHalfFov = std::tan(camera.GetFOV() / 2.0f * 3.14159265358979323846f / 180.0f);
        float zoom = 1.0f;
        if (fieldAspectRatio > windowAspectRatio) {
            zoom = width / (2.0f * tanHalfFov * windowAspectRatio);
        }
        else {
            zoom = height / (2.0f * tanHalfFov);
        }
        // Устанавливаем позицию камеры
        camera.SetPosition(width / 2.0f, height / 2.0f, zoom);
        camera.SetDirection(0.0f, 0.0f, -1.0f); // Направление вдоль оси Z (назад)
        camera.SetUpVector(0.0f, 1.0f, 0.0f); // Установка вектора "вверх"
    }
}
