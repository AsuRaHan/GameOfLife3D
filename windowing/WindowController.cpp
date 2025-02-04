#include "WindowController.h"
#include "MainWindow.h"

#include <iostream>

WindowController::WindowController(MainWindow* window, Renderer* renderer, GameController* gameController)
    : pWindow(window), pRenderer(renderer), pGameController(gameController),
    mouseCaptured(false), lastMouseX(0), lastMouseY(0), isMiddleButtonDown(false),
    gridPicker(renderer->GetCamera()) {
    ResetCamera();
}

WindowController::~WindowController() {

}


void WindowController::Resize(int width, int height) {
    if (pRenderer) {
        pRenderer->OnWindowResize(width, height);
    }
}

void WindowController::HandleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_SIZE:
        Resize(LOWORD(lParam), HIWORD(lParam)); // Обработка изменения размеров окна
        break;

    case WM_KEYDOWN:
        if (wParam >= '1' && wParam <= '9') {
            pGameController->setCurrentPattern(wParam - '0');
        }
        switch (wParam) {
            break;
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

        case VK_LEFT: // Стрелка влево для предыдущего поколения
            if (!pGameController->isSimulationRunning()) {
                pGameController->previousGeneration(); // Переход к предыдущему поколению
            }
            break;
        case VK_ADD: // Клавиша '+' на нумпаде
        case VK_OEM_PLUS: // Клавиша '+' на основной клавиатуре
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                simulationSpeedMultiplier -= 0.25f; // Уменьшаем интервал на 250 мс
            }
            else {
                simulationSpeedMultiplier -= 0.025f; // Уменьшаем интервал на 25 мс
            }
            if (simulationSpeedMultiplier < 0.01f) {
                simulationSpeedMultiplier = 0.01f; // Устанавливаем минимум 10 мс
            }
            if (pGameController->isSimulationRunning()) {
                pGameController->setSimulationSpeed(simulationSpeedMultiplier);
            }
            break;

        case VK_SUBTRACT: // Клавиша '-' на нумпаде
        case VK_OEM_MINUS: // Клавиша '-' на основной клавиатуре
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                simulationSpeedMultiplier += 0.25f; // Увеличиваем интервал на 250 мс
            }
            else {
                simulationSpeedMultiplier += 0.025f; // Увеличиваем интервал на 25 мс
            }
            if (simulationSpeedMultiplier > 0.25f) {
                simulationSpeedMultiplier = 0.25f; // Устанавливаем максимум 250 мс
            }
            if (pGameController->isSimulationRunning()) {
                pGameController->setSimulationSpeed(simulationSpeedMultiplier);
            }
            break;

        case 'R':
            if (!pGameController->isSimulationRunning()) {
                pGameController->randomizeGrid(0.1f); // Случайное заполнение поля
            }
            break;
        case 'W':
            MoveCamera(0.0f, 5.0f); // Двигаем вперед
            break;
        case 'S':
            MoveCamera(0.0f, -5.0f); // Двигаем назад
            break;
        case 'A':
            MoveCamera(-5.0f, 0.0f); // Двигаем влево
            break;
        case 'D':
            MoveCamera(5.0f, 0.0f); // Двигаем вправо
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
                pGameController->setWoldToroidal(!pGameController->getWoldToroidal()); // Очищение поля. убить всех
            }
            break;
        case 'I':
            if (!pGameController->isSimulationRunning()) {
                pGameController->initializeGrid(); // Очищение поля. убить всех
            }
            break;
        case 'L':
            if (!pGameController->isSimulationRunning()) {
                pGameController->resizeGrid(30, 30);
                pRenderer->RebuildGameField();
            }
            break;
        case 'G':
            pRenderer->setShowGrid(!pRenderer->getShowGrid());
            break;

        }
        break;

    case WM_MOUSEWHEEL:
        if (pRenderer) {
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            float moveZ = zDelta > 0 ? -1.1f : 1.1f;
            // Проверка состояния клавиши Ctrl
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                moveZ *= 10.0f; // Увеличиваем скорость зума в 10 раз
            }
            Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
            camera.Move(0.0f, 0.0f, moveZ);
        }
        break;

    case WM_LBUTTONDOWN: // Левый клик мыши
        HandleMouseClick(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_RBUTTONDOWN: // Начало захвата мыши
        SetCapture(pWindow->GetHwnd());
        mouseCaptured = true;
        lastMouseX = LOWORD(lParam);
        lastMouseY = HIWORD(lParam);
        // Скрыть курсор
        //ShowCursor(FALSE);
        break;

    case WM_RBUTTONUP: // Конец захвата мыши
        ReleaseCapture();
        mouseCaptured = false;
        // Показать курсор
        //ShowCursor(TRUE);
        break;
    case WM_MBUTTONDOWN: // Нажатие средней кнопки мыши
        PlacePattern(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_MBUTTONUP: // Отпускание средней кнопки мыши
        ReleaseCapture();
        isMiddleButtonDown = false;
        break;
    case WM_MOUSEMOVE:
        if (mouseCaptured && pRenderer) {
            int currentX = LOWORD(lParam);
            int currentY = HIWORD(lParam);

            float dx = static_cast<float>(currentX - lastMouseX) * 0.1f; // Чувствительность для перемещения
            float dy = static_cast<float>(currentY - lastMouseY) * 0.1f;

            // Перемещаем камеру
            Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
            camera.Move(dx, dy, 0.0f); // Обратите внимание на знак для Y

            lastMouseX = currentX;
            lastMouseY = currentY;
        }
        break;

    default:
        break;
    }
}

void WindowController::HandleMouseClick(int screenX, int screenY) {
    float worldX, worldY;
    gridPicker.ScreenToWorld(screenX, screenY, pRenderer->getWindowWidth(), pRenderer->getWindowHeight(), worldX, worldY);

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
        // Получаем текущее направление камеры для вычисления движения
        float dirX, dirY, dirZ;
        camera.GetDirection(dirX, dirY, dirZ);

        // Двигаем камеру вперед/назад по направлению взгляда
        float moveX = dirX * dy;
        float moveZ = dirZ * dy;

        // Двигаем камеру влево/вправо перпендикулярно направлению взгляда
        float rightX = dirZ;
        float rightZ = -dirX;
        moveX += rightX * dx;
        moveZ += rightZ * dx;

        // Применяем движение
        camera.Move(moveX, 0.0f, moveZ);
    }
}

void WindowController::PlacePattern(int screenX, int screenY) {
    float worldX, worldY;
    gridPicker.ScreenToWorld(screenX, screenY, pRenderer->getWindowWidth(), pRenderer->getWindowHeight(), worldX, worldY);

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
        float windowAspectRatio = static_cast<float>(pRenderer->getWindowWidth()) / pRenderer->getWindowHeight();
        float fieldAspectRatio = width / height;

        float zoom = 1.0f;
        if (fieldAspectRatio > windowAspectRatio) {
            zoom = width / (2.0f * std::tan(camera.GetFOV() / 2.0f * 3.14159265358979323846f / 180.0f) * pRenderer->getWindowWidth() / pRenderer->getWindowHeight());
        }
        else {
            zoom = height / (2.0f * std::tan(camera.GetFOV() / 2.0f * 3.14159265358979323846f / 180.0f));
        }

        // Устанавливаем позицию камеры
        camera.SetPosition(width / 2.0f, height / 2.0f, zoom);
        camera.SetDirection(0.0f, 0.0f, -1.0f); // Направление вдоль оси Z (назад)
        camera.SetUpVector(0.0f, 1.0f, 0.0f); // Установка вектора "вверх"
    }
}
