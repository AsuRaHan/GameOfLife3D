#version 430 core
layout(local_size_x = {{groupSizeX}}, local_size_y = {{groupSizeY}}) in;

layout(std430, binding = 0) buffer CurrentCells {
    int current[]; // Массив текущих состояний клеток
};

layout(std430, binding = 1) buffer NextCells {
    int next[]; // Массив состояний клеток на следующем шаге
};

layout(std430, binding = 2) buffer Colors {
    vec4 colors[]; // Массив цветов клеток (RGB - цвет, A - энергия)
};

uniform ivec2 gridSize; // Размеры игрового поля (ширина и высота)
uniform int neighborhoodRadius; // Радиус окрестности для подсчета соседей
uniform bool isToroidal; // Тороидальная поверхность (закольцованная) или нет

uniform int birth; // Количество соседей для рождения клетки (стандартный режим)
uniform int survivalMin; // Минимальное количество соседей для выживания клетки (стандартный режим)
uniform int survivalMax; // Максимальное количество соседей для выживания клетки (стандартный режим)
uniform int overpopulation; // Перенаселение (стандартный режим)

uniform int birthCounts[9]; // Массив правил рождения (расширенный режим)
uniform int surviveCounts[9]; // Массив правил выживания (расширенный режим)
uniform int overpopulationCounts[9]; // Массив правил перенаселения (расширенный режим)
uniform int useAdvancedRules; // 0 - стандартный режим, 1 - расширенный режим

uniform int starvingTicks = 10; // Количество шагов без еды, после которых хищник умирает (не используется напрямую, но может быть в расширенном режиме)
uniform float mutantChance = 0.0005; // Шанс мутации клетки в другой тип (от 0.0 до 1.0)

uniform float reproduceEnergyThreshold = 0.8; // Порог энергии для размножения
uniform float energyCostPerTick = 0.001; // Стоимость жизни за тик
uniform float reproduceEnergyCost = 0.4; // Цена размножения
uniform float energyPerPrey  = 0.2; // Сколько энергии получает хищник за жертву
uniform float energyAbsorbed = 0.005; // Поглощения энергии клетками за тик
uniform float startEnergy = 0.5;//Стартовое значение энергии для клетки
uniform float energyFromDead = 0.15;// Сколько энергии получает падальщик за мертвую клетку.

// Зелёные: Жертвы.
// Красные: Хищники.
// Голубые: Растения.
// Серые: Травоядные.
// Фиолетовые: Падальщики.
// Желтые, белые: Мутанты.
// Белые: Мины.
// Темно-серые: Мёртвые клетки.
// Черные: Пустые клетки.
// Интенсивность: Чем ярче цвет, тем больше энергии у клетки.

//---------------------------------------------------------
// --- СИСТЕМА ПРАВИЛ И ПОВЕДЕНИЯ КЛЕТОК ---
//---------------------------------------------------------

// Проверяет, может ли клетка родиться в расширенном режиме
bool canBeBorn(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && birthCounts[neighbors] == 1;
}

// Проверяет, может ли клетка выжить в расширенном режиме
bool canSurvive(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && surviveCounts[neighbors] == 1;
}

// Проверяет, умирает ли клетка от перенаселения в расширенном режиме
bool isOverpopulated(int neighbors) {
    return neighbors >= 0 && neighbors <= 8 && overpopulationCounts[neighbors] == 1;
}

//---------------------------------------------------------
// --- СИСТЕМА ПОДСЧЕТА СОСЕДЕЙ ---
//---------------------------------------------------------

// Подсчитывает живых соседей вокруг клетки
// pos - позиция клетки
// targetType - тип соседей, которых нужно считать (0 - все живые, >0 - конкретный тип)
int countLiveNeighbors(ivec2 pos, int targetType) {
    int count = 0;
    for (int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) {
        for (int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) {
            if (dx == 0 && dy == 0) continue; // Игнорируем саму клетку
            ivec2 neighbor = pos + ivec2(dx, dy); // Позиция соседней клетки

            // Проверка на тороидальность (замкнутость поля)
            if (isToroidal) {
                neighbor = (neighbor + gridSize) % gridSize;
            } else {
                // Проверка на выход за границы поля
                if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
            }

            int neighborState = current[neighbor.y * gridSize.x + neighbor.x]; // Получаем состояние соседней клетки

            // Если нужен подсчет всех живых соседей
            if (targetType == 0) {
                count += (neighborState > 0) ? 1 : 0;
            } else {
                // Если нужен подсчет соседей определенного типа
                count += (neighborState == targetType) ? 1 : 0;
            }
        }
    }
    return count;
}
//---------------------------------------------------------
// --- СИСТЕМА ПОДСЧЕТА МЕРТВЫХ СОСЕДЕЙ ---
//---------------------------------------------------------
int countDeadNeighbors(ivec2 pos) {
    int count = 0;
    for (int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) {
        for (int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) {
            if (dx == 0 && dy == 0) continue;
            ivec2 neighbor = pos + ivec2(dx, dy);
            if (isToroidal) {
                neighbor = (neighbor + gridSize) % gridSize;
            } else {
                if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
            }
            int neighborState = current[neighbor.y * gridSize.x + neighbor.x];
            if (neighborState == 0) { // Считаем мертвых соседей
                count++;
            }
        }
    }
    return count;
}

//---------------------------------------------------------
// --- СИСТЕМА ОПРЕДЕЛЕНИЯ ТИПА НОВОЙ КЛЕТКИ ---
//---------------------------------------------------------
// Определяет тип новорожденной клетки на основе типов её соседей
int determineNewType(ivec2 pos) {
    int neighborsCount[8];
    for (int i = 1; i <= 7; i++) {
        neighborsCount[i] = countLiveNeighbors(pos, i); // Подсчитываем количество соседей каждого типа
    }

    int maxCount = 0;
    int equalTypes[8];
    int equalCount = 0;
    for (int i = 1; i <= 7; i++) {
        // Ищем тип с максимальным количеством соседей
        if (neighborsCount[i] > maxCount) {
            maxCount = neighborsCount[i];
            equalCount = 0;
            equalTypes[equalCount] = i; // Сохраняем новый макс. тип
            equalCount++;
        } else if (neighborsCount[i] == maxCount && maxCount != 0) {
            // Если есть несколько типов с одинаковым максимальным количеством
            equalTypes[equalCount] = i; // Сохраняем равный тип
            equalCount++;
        }
    }
    if (maxCount == 0) return 0; //если нет соседей, то умирает

    // Если есть несколько типов с равным максимальным количеством соседей, выбираем случайный
    if (equalCount > 1) {
        int randomIndex = int(float(equalCount) * fract(sin(dot(pos, vec2(12.9898, 78.233))) * 43758.5453));
        return equalTypes[randomIndex]; // Возвращаем случайный тип из равных
    } else {
        return equalTypes[0]; // Возвращаем единственный максимальный тип
    }
}

//---------------------------------------------------------
// --- СИСТЕМА ВИЗУАЛИЗАЦИИ КЛЕТОК ---
//---------------------------------------------------------
// Метод для генерации цвета на основе типа клетки.
vec4 getColorByType(int type, int currentState, float energy) {
    if (type == 0) {
        return vec4(0.05, 0.05, 0.08, 0.0); // Мертвая клетка (темно-серый)
    }
    if (type < 0) {
        return vec4(0.0, 0.0, 0.0, 0.0); // Пустая клетка
    }

    vec4 color = vec4(0.0, 0.0, 0.0, energy);
    float increase = 0.02;

    //Цвет для жертвы
    if(type == 1){
        color = vec4(0.0, 1.0 * energy, 0.0, energy); // Жертва (зеленый)
    }

    //Цвет для хищника
    if(type == 2){
        color = vec4(1.0 * energy, 0.0, 0.0, energy); // Хищник (красный)
    }
    //Цвет для растений
    if(type == 3){
        color = vec4(0.0, 1.0 * energy, 1.0 * energy, energy); // Растение (голубой)
    }
    //Цвет травоядного
     if(type == 4){
        color = vec4(0.5 * energy, 0.5 * energy, 0.5 * energy, energy); // Травоядное (серый)
    }
    //Цвет для падальщика
    if(type == 5){
        color = vec4(0.5 * energy, 0.0, 0.5 * energy, energy); // Падальщик (фиолетовый)
    }
    if (type > 5 && type <= 7){
         int t = type;
        int r = (t / 4) % 2;
        int g = (t / 2) % 2;
        int b = (t / 1) % 2;

        color.r = float(r) * energy;
        color.g = float(g) * energy;
        color.b = float(b) * energy;
        color.a = energy; // сохраняем энергию в альфа канал
    }

     if(type > 7){
        color = vec4(1.0 * energy,1.0 * energy,1.0 * energy,energy); //Плавающая мина
    }
    // Эффект постепенного усиления цвета
    if(currentState > 0 && currentState == type){
        vec4 currentColor = colors[gl_GlobalInvocationID.y * gridSize.x + gl_GlobalInvocationID.x];
        color.r = min(currentColor.r + increase, color.r);
        color.g = min(currentColor.g + increase, color.g);
        color.b = min(currentColor.b + increase, color.b);
    }
    if(type > 0){
        color = clamp(color,vec4(0,0,0,0),vec4(1,1,1,1)); //Ограничиваем цветовой диапазон
    }
    return color;
}

//---------------------------------------------------------
// --- СИСТЕМА ПЕРЕМЕЩЕНИЯ ---
//---------------------------------------------------------
//Метод для перемещения клеток к цели
ivec2 canMoveTo(ivec2 pos, int targetType){
    ivec2 targetPos = ivec2(0);
    int minDistance = 10000;
    //ищем всех жертв в пределах досягаемости
    for (int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) {
        for (int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) {
            if(dx == 0 && dy == 0) continue;
            ivec2 neighbor = pos + ivec2(dx, dy);
            if (isToroidal) {
                neighbor = (neighbor + gridSize) % gridSize;
            } else {
                if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
            }
            if (current[neighbor.y * gridSize.x + neighbor.x] == targetType){
                int distance = abs(dx)+abs(dy); //вычисляем растояние до жертвы
                if(distance < minDistance){
                    minDistance = distance;
                    targetPos = neighbor;
                }
            }
        }
    }
    if(targetPos == ivec2(0)) return targetPos;//цель не найдена
    return targetPos;

}

//---------------------------------------------------------
// --- ГЛАВНЫЙ МЕТОД ---
//---------------------------------------------------------

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    // Проверка на выход за границы поля
    if (pos.x >= gridSize.x || pos.y >= gridSize.y) return;

    int index = pos.y * gridSize.x + pos.x; // Индекс текущей клетки
    int currentState = current[index]; // Текущее состояние клетки
    int neighbors = countLiveNeighbors(pos, 0); // Все живые соседи
    int nextState = currentState; // Состояние клетки на следующем шаге
    float currentEnergy = colors[index].a;//получаем текущию энергию клетки
    float nextEnergy = currentEnergy;
    //---------------------------------------------------------
    // --- ЛОГИКА ХИЩНИКОВ ---
    //---------------------------------------------------------
    if (currentState == 2) { // Если текущая клетка - хищник
        int preyCount = countLiveNeighbors(pos, 1); // Сколько жертв вокруг
        
        if (preyCount > 0) {
            // Если жертва рядом, съедаем ее (превращаем в 0)
            for (int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) {
                for (int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) {
                    if(dx == 0 && dy == 0) continue;
                    ivec2 neighbor = pos + ivec2(dx, dy);
                    if (isToroidal) {
                        neighbor = (neighbor + gridSize) % gridSize;
                    } else {
                        if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
                    }
                    if (current[neighbor.y * gridSize.x + neighbor.x] == 1){ // Проверяем является ли клетка жертвой
                        next[neighbor.y * gridSize.x + neighbor.x] = 0; // Помечаем клетку жертву на удаление
                        nextEnergy += energyPerPrey;// получаем энергию за жертву
                        nextEnergy = min(nextEnergy,1.0);//не даем энергии быть больше 1
                    }
                }
            }
        } else {
            nextEnergy -= energyCostPerTick; // уменьшаем энергию за жизнь
            if(nextEnergy < 0.0){
                nextState = 0; // Умирает от голода
                nextEnergy = 0.0;
            }
        }
        // Перемещение хищника к жертве.
        ivec2 target = canMoveTo(pos, 1);
        if(target.x != 0 && next[target.y * gridSize.x + target.x] != 0) {
             next[target.y * gridSize.x + target.x] = nextState; // Помещаем его в новую клетку
             colors[target.y * gridSize.x + target.x].a = nextEnergy;
            nextState = 0;//оставляем пустоту за хищником
            nextEnergy = 0.0;//оставляем пустоту за хищником
        }
        
    }

        //---------------------------------------------------------
        // --- ЛОГИКА ТРАВОЯДНОГО ---
        //---------------------------------------------------------
        if (currentState == 4) {
             int preyCount = countLiveNeighbors(pos, 3); // Сколько растений вокруг
            if (preyCount > 0) {
                 // Если растение рядом, съедаем ее (превращаем в 0)
                for (int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) {
                    for (int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) {
                        if(dx == 0 && dy == 0) continue;
                        ivec2 neighbor = pos + ivec2(dx, dy);
                        if (isToroidal) {
                            neighbor = (neighbor + gridSize) % gridSize;
                        } else {
                            if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
                        }
                        if (current[neighbor.y * gridSize.x + neighbor.x] == 3){ // Проверяем является ли клетка растением
                            next[neighbor.y * gridSize.x + neighbor.x] = 0; // Помечаем клетку
                            nextEnergy += energyPerPrey; // получаем энергию за съеденное растение
                            nextEnergy = min(nextEnergy, 1.0); //ограничиваем количество энергии
                        }
                    }
                }
            } else {
                 nextEnergy -= energyCostPerTick; // уменьшаем энергию за жизнь
                if(nextEnergy < 0.0){
                    nextState = 0; // Умирает от голода
                    nextEnergy = 0.0;
                }
            }
             // Перемещение травоядного к растению.
            ivec2 target = canMoveTo(pos, 3);
            if(target.x != 0 && next[target.y * gridSize.x + target.x] != 0) {
                next[target.y * gridSize.x + target.x] = nextState; // Помещаем его в новую клетку
                colors[target.y * gridSize.x + target.x].a = nextEnergy;
                nextState = 0;//оставляем пустоту за травоядным
                nextEnergy = 0.0;
            }
        }
        //---------------------------------------------------------
        // --- ЛОГИКА ПАДАЛЬЩИКА ---
        //---------------------------------------------------------
         if (currentState == 5) {
            int deadCount = countDeadNeighbors(pos);
            if (deadCount > 0) {
                 for (int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) {
                    for (int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) {
                        if(dx == 0 && dy == 0) continue;
                        ivec2 neighbor = pos + ivec2(dx, dy);
                        if (isToroidal) {
                            neighbor = (neighbor + gridSize) % gridSize;
                        } else {
                            if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
                        }
                        if (current[neighbor.y * gridSize.x + neighbor.x] == 0){ // Проверяем является ли клетка мертвой
                            next[neighbor.y * gridSize.x + neighbor.x] = -1; // Помечаем клетку как обработанную
                            nextEnergy += energyFromDead; // получаем энергию за мертвую клетку
                             nextEnergy = min(nextEnergy, 1.0); //ограничиваем количество энергии
                        }
                    }
                }

            }else {
                nextEnergy -= energyCostPerTick; // уменьшаем энергию за жизнь
                 if(nextEnergy < 0.0){
                    nextState = 0; // Умирает от голода
                     nextEnergy = 0.0;
                }
            }
             //перемещения к трупу не делаем так как их надо есть на месте.
         }
        //---------------------------------------------------------
        // --- ЛОГИКА ЭНЕРГЕТИЧЕСКОГО РАСТЕНИЯ ---
        //---------------------------------------------------------
        if (currentState == 3) {
            nextEnergy += energyAbsorbed; // поглощаем энергию
             nextEnergy = min(nextEnergy, 1.0); //ограничиваем количество энергии
        }

        //---------------------------------------------------------
        // --- ПОВЕДЕНИЕ МИНЫ ---
        //---------------------------------------------------------
        if(currentState > 7){
            for (int dy = -neighborhoodRadius; dy <= neighborhoodRadius; ++dy) {
                for (int dx = -neighborhoodRadius; dx <= neighborhoodRadius; ++dx) {
                    if(dx == 0 && dy == 0) continue;
                    ivec2 neighbor = pos + ivec2(dx, dy);
                    if (isToroidal) {
                        neighbor = (neighbor + gridSize) % gridSize;
                    } else {
                        if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
                    }
                    if(current[neighbor.y * gridSize.x + neighbor.x] != currentState && current[neighbor.y * gridSize.x + neighbor.x] != 0 && current[neighbor.y * gridSize.x + neighbor.x] > 0){
                      next[neighbor.y * gridSize.x + neighbor.x] = 0; //уничтожаем всё на своём пути
                      nextState = 0;//самоуничтожение
                      nextEnergy = 0.0;
                    }
                }
            }
        }

        //---------------------------------------------------------
        // --- ЛОГИКА РАЗМНОЖЕНИЯ ---
        //---------------------------------------------------------
        if (currentState > 0 && nextEnergy >= reproduceEnergyThreshold) {
                nextEnergy -= reproduceEnergyCost; // тратим энергию на размножение

                 // Проверяем наличие свободных соседних клеток
                bool hasEmptyNeighbor = false;
                ivec2 emptyNeighborPos = ivec2(-1); // Инициализируем вне границ
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        ivec2 neighbor = pos + ivec2(dx, dy);
                        if (isToroidal) {
                            neighbor = (neighbor + gridSize) % gridSize;
                        } else {
                            if (neighbor.x < 0 || neighbor.x >= gridSize.x || neighbor.y < 0 || neighbor.y >= gridSize.y) continue;
                        }
                        int neighborState = current[neighbor.y * gridSize.x + neighbor.x];
                        if (neighborState <= 0) { //нашли свободную клетку
                            hasEmptyNeighbor = true;
                            emptyNeighborPos = neighbor;
                           break;
                        }
                    }
                    if(hasEmptyNeighbor) break;
                }

                if (hasEmptyNeighbor) {
                    // Создаём дочернюю клетку в пустом соседнем месте
                    next[emptyNeighborPos.y * gridSize.x + emptyNeighborPos.x] = currentState;
                    colors[emptyNeighborPos.y * gridSize.x + emptyNeighborPos.x].a = nextEnergy/2;//половина энергии
                    nextEnergy /= 2.0;
                }
        }

        //---------------------------------------------------------
        // --- ЛОГИКА МУТАЦИЙ ---
        //---------------------------------------------------------
        if (currentState > 0 && currentState <= 7) {
            float rnd = fract(sin(dot(pos, vec2(12.9898, 78.233))) * 43758.5453); // Генерируем случайное число от 0 до 1.0
            if (rnd < mutantChance) {
                nextState = int(rnd * 4.0) + 3; // Мутируем в случайный тип от 3 до 7
            }
        }

        //---------------------------------------------------------
        // --- СТАНДАРТНАЯ ЛОГИКА КЛЕТОК ---
        //---------------------------------------------------------
    if(currentState > 0 && currentState <= 7){
        nextEnergy -= energyCostPerTick; // уменьшаем энергию за жизнь
        if(nextEnergy < 0.0){
            nextState = 0; // Умирает от голода
             nextEnergy = 0.0;
        }
    }

    if (useAdvancedRules == 1) {
        // Расширенный режим (B/S)
        if (currentState > 0 && currentState <= 7) {
            if (canSurvive(neighbors) && !isOverpopulated(neighbors)) nextState = currentState;
            else nextState = 0;
        } else if (canBeBorn(neighbors)) {
             nextState = determineNewType(pos);
        }
    } else {
        // обычный режим
        if (currentState > 0 && currentState <= 7) { // Живая клетка
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
    if(nextState > 0){
        if(currentState <= 0){
            nextEnergy = startEnergy;
        }
    }
    next[index] = nextState; // Обновляем состояние клетки на следующем шаге
    colors[index] = getColorByType(nextState, currentState, nextEnergy); // Обновляем цвет и энергию клетки
}
