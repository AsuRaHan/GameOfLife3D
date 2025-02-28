#version 430 core
layout(local_size_x = {{groupSizeX}}, local_size_y = {{groupSizeY}}) in;

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

uniform int birthCounts[9];
uniform int surviveCounts[9];
uniform int useAdvancedRules; // 0 - стадартный режим, 1 - расширенный
uniform int overpopulationCounts[9];

bool canBeBorn(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && birthCounts[neighbors] == 1;
}

bool canSurvive(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && surviveCounts[neighbors] == 1;
}

bool isOverpopulated(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && overpopulationCounts[neighbors] == 1;
}

int countLiveNeighbors(ivec2 pos, int targetType) {
    int count = 0;
    for(int dy = -2; dy <= 2; ++dy) {
        for(int dx = -2; dx <= 2; ++dx) {
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

// Метод для генерации цвета на основе типа клетки.
vec4 getColorByType(int type, int currentState) {
    if (type == 0) {
        return vec4(0.05, 0.05, 0.08, 0.0); // Мертвая клетка
    }
    if (type < 0) {
        return vec4(0.0, 0.0, 0.0, 0.0); // Пустая клетка
    }


    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
    float increase = 0.02;

    int t = type;
    int r = (t / 4) % 2;
    int g = (t / 2) % 2;
    int b = (t / 1) % 2;

    color.r = float(r);
    color.g = float(g);
    color.b = float(b);

    if(currentState > 0 && currentState == type){
        vec4 currentColor = colors[gl_GlobalInvocationID.y * gridSize.x + gl_GlobalInvocationID.x];
        color.r = min(currentColor.r + increase, color.r);
        color.g = min(currentColor.g + increase, color.g);
        color.b = min(currentColor.b + increase, color.b);
    }
    if (type > 7){
        color = vec4(1.0,1.0,1.0,0.0);
    }
    if(type > 0){
        color = clamp(color,vec4(0,0,0,0),vec4(1,1,1,0));
    }
    return color;
}


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;

    int index = pos.y * gridSize.x + pos.x;
    int currentState = current[index];
    int neighbors = countLiveNeighbors(pos, 0); // Все живые соседи
    int nextState = currentState;

    if (useAdvancedRules == 1) {
        // Расширенный режим (B/S)
        if (currentState > 0) {
            if (canSurvive(neighbors) && !isOverpopulated(neighbors)) nextState = currentState;
            else nextState = 0;
        } else if (canBeBorn(neighbors)) {
            nextState = determineNewType(pos);
        }
    } else {
        // обычный режим
        if (currentState > 0) { // Живая клетка
            if (neighbors >= survivalMin && neighbors <= survivalMax && neighbors < overpopulation) {
                nextState = currentState; // Сохраняем тип
            } else {
                nextState = 0; // Умирает
            }
        } else if (currentState <= 0) { // Мёртвая клетка
            if (neighbors == birth) {
                nextState = determineNewType(pos); // Оживает с типом
            }
        }
    }

    next[index] = nextState;
    colors[index] = getColorByType(nextState, currentState);
}