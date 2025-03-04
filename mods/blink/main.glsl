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

uniform int altbirth;

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
    if(altbirth > 2) return 1;
    int neighborsCount[8];
    for (int i = 1; i <= 7; i++) {
        neighborsCount[i] = countLiveNeighbors(pos, i);
    }

    int maxCount = 0;
    int equalTypes[8];
    int equalCount = 0;
    for (int i = 1; i <= 7; i++) {
        if (neighborsCount[i] > maxCount) {
            maxCount = neighborsCount[i];
            equalCount = 0;
            equalTypes[equalCount] = i; // Сохраняем новый макс. тип
            equalCount++;
        } else if (neighborsCount[i] == maxCount && maxCount != 0) {
            equalTypes[equalCount] = i; // Сохраняем равный тип
            equalCount++;
        }
    }
    if(maxCount == 0) return 0; //если нет соседей, то умирает

    if (equalCount > 1) {
        int randomIndex = int(float(equalCount) * fract(sin(dot(pos, vec2(12.9898, 78.233))) * 43758.5453));
        return equalTypes[randomIndex]; // Возвращаем случайный тип из равных
    } else {
        return equalTypes[0]; // Возвращаем единственный максимальный тип
    }
}

vec4 getColorByType(int type) {
    if (type == 0) {
        return vec4(0.05, 0.05, 0.08, 0.0); // Мертвая клетка
    }
    if (type < 0) {
        return vec4(0.0, 0.0, 0.0, 0.0); // Пустая клетка
    }

    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    // Используем 4 бита для определения цвета
    int r = (type >> 2) & 1; // 1 бит для красного
    int g = (type >> 1) & 1; // 1 бит для зеленого
    int b = (type >> 0) & 1; // 1 бит для синего

    // Увеличиваем количество цветов, добавляя дополнительные биты
    if (type >= 8) {
        r = (type >> 3) & 1; // 1 дополнительный бит для красного
        g = (type >> 4) & 1; // 1 дополнительный бит для зеленого
        b = (type >> 5) & 1; // 1 дополнительный бит для синего
    }

    color.r = float(r);
    color.g = float(g);
    color.b = float(b);

    return color;
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
    colors[index] = getColorByType(nextState);
}
