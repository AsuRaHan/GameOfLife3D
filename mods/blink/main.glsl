#version 430 core
layout(local_size_x = {{groupSizeX}}, local_size_y = {{groupSizeY}}) in;
// modName=blink
layout(std430, binding = 0) buffer CurrentCells {
    int current[];
};

layout(std430, binding = 1) buffer NextCells {
    int next[];
};

layout(std430, binding = 2) buffer Colors {
    vec4 colors[];
};

uniform ivec2 gridSize;
uniform int neighborhoodRadius;
uniform bool isToroidal;

uniform int birth;
uniform int survivalMin;
uniform int survivalMax;
uniform int overpopulation;

int countLiveNeighbors(ivec2 pos, int targetType) {
    int count = 0;
    for(int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) {
        for(int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) {
            if(dx == 0 && dy == 0) continue;
            ivec2 neighbor = pos + ivec2(dx, dy);
            if (isToroidal) {
                neighbor = (neighbor + gridSize) % gridSize;
            } else {
                if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
            }
            int neighborState = current[neighbor.y * gridSize.x + neighbor.x];
            if (targetType == 0) { // Все живые
                count += (neighborState > 0) ? 1 : 0;
            } else { // Конкретный тип
                count += (neighborState == targetType) ? 1 : 0;
            }
        }
    }
    return count;
}

int determineNewType(ivec2 pos) {
    return 1;
}

vec4 getColorByType(int type, int currentState) {
    if (type == 0) {
        return vec4(0.05, 0.05, 0.08, 0.0); // Мертвая клетка
    }
    return vec4(1.0, 0.0, 0.0, 0.0); // Живая клетка - красная
}

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;

    int index = pos.y * gridSize.x + pos.x;
    int currentState = current[index];
    int neighbors = countLiveNeighbors(pos, 0);
    int nextState = currentState;

    // обычный режим
    if (currentState > 0) { // Живая клетка
        if (neighbors >= survivalMin && neighbors <= survivalMax && neighbors < overpopulation) {
            nextState = currentState; // Сохраняем тип
        } else {
            nextState = 0; // Умирает
        }
    } else if (neighbors == birth) { // Мёртвая клетка
        nextState = determineNewType(pos); // Оживает с типом
    }
    next[index] = nextState;
    colors[index] = getColorByType(nextState, currentState);
}
