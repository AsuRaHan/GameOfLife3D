#pragma once
#ifndef INPUTHEVENT_H_
#define INPUTHEVENT_H_

struct InputEvent {
    enum class InputType { None, MouseMove, MouseWheel, KeyDown, MouseButtonDown, MouseButtonUp };
    enum class MouseButton { Left, Right, Middle };

    InputType type;
    int key; // Код клавиши для KeyEvent
    float deltaX, deltaY; // Delta движения мыши
    float wheelDelta; // Дельта прокрутки колесика мыши
    MouseButton button; // Для событий мыши
};

#endif // INPUTHEVENT_H_