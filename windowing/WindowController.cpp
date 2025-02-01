#include "WindowController.h"
#include "MainWindow.h"

#include <iostream>

WindowController::WindowController(MainWindow* window, Renderer* renderer, GameController* gameController)
    : pWindow(window), pRenderer(renderer), pGameController(gameController),
    mouseCaptured(false), lastMouseX(0), lastMouseY(0), isMiddleButtonDown(false),
    gridPicker(renderer->GetCamera(), *gameController) {

    // Установка таймера для обновления симуляции
    timerId = SetTimer(pWindow->GetHwnd(), 1, 100, NULL); // Интервал 100 мс
    if (!timerId) {
        MessageBox(NULL, L"Timer could not be set", L"Error", MB_OK | MB_ICONERROR);
    }
}

WindowController::~WindowController() {
    if (timerId) {
        KillTimer(pWindow->GetHwnd(), timerId);
    }
}

void WindowController::Resize(int width, int height) {
    if (pRenderer) {
        pRenderer->OnWindowResize(width, height);
    }
}

void WindowController::HandleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_TIMER:
        if (wParam == timerId) {
            pGameController->update(); // Обновляем симуляцию
        }
        break;

    case WM_SIZE:
        Resize(LOWORD(lParam), HIWORD(lParam)); // Обработка изменения размеров окна
        break;

    case WM_KEYDOWN:
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

        case VK_LEFT: // Стрелка влево для предыдущего поколения
            if (!pGameController->isSimulationRunning()) {
                pGameController->previousGeneration(); // Переход к предыдущему поколению
            }
            break;
        case 'R': // Клавиша 'R' для случайного заполнения поля
            if (!pGameController->isSimulationRunning()) {
                pGameController->randomizeGrid(0.5f); // Случайное заполнение поля
            }
            break;
        case 'W':
            MoveCamera(0.0f, 1.0f); // Двигаем вперед
            break;
        case 'S':
            MoveCamera(0.0f, -1.0f); // Двигаем назад
            break;
        case 'A':
            MoveCamera(-1.0f, 0.0f); // Двигаем влево
            break;
        case 'D':
            MoveCamera(1.0f, 0.0f); // Двигаем вправо
            break;
        case 'T':
            ResetCamera();
            break;
        }
        break;

    case WM_MOUSEWHEEL:
        if (pRenderer) {
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            float moveZ = zDelta > 0 ? -1.1f : 1.1f;
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
        SetCapture(pWindow->GetHwnd());
        isMiddleButtonDown = true;
        lastMouseX = LOWORD(lParam);
        lastMouseY = HIWORD(lParam);
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
        //if (isMiddleButtonDown && pRenderer) {
        //    int currentX = LOWORD(lParam);
        //    int currentY = HIWORD(lParam);

        //    float dx = static_cast<float>(currentX - lastMouseX) * 0.005f; // Чувствительность для поворота по YAW
        //    float dy = static_cast<float>(currentY - lastMouseY) * 0.005f; // Чувствительность для поворота по PITCH

        //    RotateCamera(dx, dy);

        //    lastMouseX = currentX;
        //    lastMouseY = currentY;
        //}
        break;

    default:
        break;
    }
}

void WindowController::HandleMouseClick(int screenX, int screenY) {
    if (pGameController && pRenderer) {
        gridPicker.HandleClick(screenX, screenY, pRenderer->getWindowWidth(), pRenderer->getWindowHeight());
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