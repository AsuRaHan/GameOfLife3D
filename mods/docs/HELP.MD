# Compute Shader для Игры Жизни (Game of Life)

## modystemshader.glsl
Этот раздел содержит compute shader на языке HLSL, который реализует клеточный автомат "Игра Жизни" (Game of Life) с расширенными возможностями. это стандартный шейдер который используется по умолчанию. здесь он предоставлен как ознакомительный.

## Особенности

*   **Множество типов клеток:** В отличие от классической "Игры Жизни", в этой реализации клетки могут иметь разные типы (от 0 до 7+), а не только "живая" или "мертвая".
*   **Два режима правил:**
    *   **Обычный режим:** Стандартные правила "Игры Жизни" с параметрами `birth`, `survivalMin`, `survivalMax` и `overpopulation`.
    *   **Расширенный режим:** Позволяет гибко настраивать правила рождения, выживания и перенаселения для каждого количества соседей. Правила задаются с помощью массивов `birthCounts`, `surviveCounts` и `overpopulationCounts`.
*   **Динамическое определение типа новорожденных клеток:** Когда клетка рождается в расширенном или обычном режиме, её тип определяется на основе типов её соседей. Если у клетки несколько типов соседей, в равном количестве, то выбирается случайный тип.
*   **Поддержка тороидальной/не-тороидальной поверхности:** Можно настроить, будет ли поле замкнутым (тороидальным) или нет, с помощью параметра `isToroidal`.
*   **Визуализация на основе типа:** Цвет клетки динамически меняется в зависимости от её типа и текущего состояния, обеспечивая наглядное отображение происходящего. Присутствует эффект постепенного усиления цвета клетки.
* **Смерть от перенаселения:** Клетка может умирать если количество её живых соседей больше указанного значения `overpopulation`. Также в расширенном режиме можно настроить смерти от перенаселения на каждый тип.
*   **Настраиваемый радиус окрестности:** Количество учитываемых соседей регулируется параметром `neighborhoodRadius`.

## Использование

Шейдер предназначен для использования в качестве compute shader в ModSystemAutomaton.

### Входные данные
Из вне шейдер может получить только эти параметры.

*   `CurrentCells`: Буфер с текущим состоянием поля (массив целых чисел).
* `NextCells`: Буфер для записи состояния поля на следующем шаге (массив целых чисел).
* `Colors`: Буфер для хранения цветов клеток (массив `vec4`).
* `gridSize`: Размеры поля ( `ivec2`).
*   `neighborhoodRadius`: Радиус окрестности для подсчета соседей ( `int`).
*   `isToroidal`: Тороидальная поверхность или нет (`bool`).
*   `birth`: Количество соседей для рождения клетки в обычном режиме ( `int`).
*   `survivalMin`, `survivalMax`: Минимальное и максимальное количество соседей для выживания клетки в обычном режиме (`int`).
*   `overpopulation`: Если живых соседей больше этого значения, клетка умирает в обычном режиме.(`int`)
*   `birthCounts`, `surviveCounts`, `overpopulationCounts`: Массивы правил для расширенного режима (`int[9]`).
*   `useAdvancedRules`: Флаг переключения между обычным и расширенным режимами (0 - обычный, 1 - расширенный) (`int`).

### Выходные данные

*   `NextCells`: Обновленное состояние поля на следующем шаге.
*   `Colors`: Обновленные цвета клеток.

## Описание функций

*   `canBeBorn(int neighbors)`: Проверяет, может ли клетка родиться в расширенном режиме.
*   `canSurvive(int neighbors)`: Проверяет, может ли клетка выжить в расширенном режиме.
* `isOverpopulated(int neighbors)`: Проверяет, умирает ли клетка от перенаселения в расширенном режиме.
*   `countLiveNeighbors(ivec2 pos, int targetType)`: Подсчитывает живых соседей вокруг клетки в позиции `pos`. `targetType` позволяет фильтровать соседей по типу, если передан 0, то считает всех живых.
*   `determineNewType(ivec2 pos)`: Определяет тип новой клетки на основе типов её соседей.
*   `getColorByType(int type, int currentState)`: Генерирует цвет клетки на основе её типа.
*   `main()`: Основная функция шейдера, которая обновляет состояние поля и цвета клеток.

# Как стать Волшебником Миров и создавать свои Моды!

## genesis.glsl
Привет, юные исследователи и творцы! Вы уже знаете, что такое "Игра Жизни"? Это такой мир, где маленькие клеточки живут по своим правилам, рождаются, растут, размножаются и умирают. А ещё там есть хищники, жертвы, растения и даже плавающие мины! Круто, правда?

Но самое крутое, что вы можете сами стать Волшебниками Миров и создавать свои собственные правила! Вы можете придумать новых клеточных жителей, изменить их поведение и сделать свой неповторимый мир!  Это называется "Мод"!

**Что нам понадобится?**

1.  **Немного смелости и воображения!** Это самое главное.
2.  **Компьютер, на котором есть наша "Игра Жизни".**
3.  **Текстовый редактор:** Например, "Блокнот" (он есть на каждом компьютере).
4.  **Внимательность:** Надо будет внимательно читать и повторять то, что написано в инструкции.

**Шаг 1: Где живут наши Волшебные Правила?**

Все правила для нашего мира хранятся в специальном файле, который называется `genesis.hlsl`.  Это как волшебная книга с заклинаниями для нашего мира. Этот файл находится в папке `shader`.

**Шаг 2: Открываем Волшебную Книгу.**

Найдите файл `genesis.hlsl` и откройте его в текстовом редакторе. Не пугайтесь, там много всяких слов, но мы сейчас во всём разберёмся!

**Шаг 3: Познакомимся с жителями.**

В нашей волшебной книге уже есть описание разных жителей:

*   **Жертвы (тип 1):** Зелёные клеточки. Они просто живут и стараются размножаться.
*   **Хищники (тип 2):** Красные клеточки. Они охотятся на жертв.
*   **Энергетические Растения (тип 3):** Голубые клеточки. Они накапливают энергию.
*   **Травоядные (тип 4):** Серые клеточки. Они кушают растения.
* **Падальщики (тип 5)**: Фиолетовые клеточки. Они едят мёртвые клетки.
*   **Мутанты (типы 6 и 7):** Клеточки разных цветов. Они могут появляться случайно.
*   **Плавающие Мины (тип 8 и больше):** Белые клетки. Очень опасные! Уничтожают всё вокруг.
* **Мёртвые клетки (тип 0):** Темно серые клетки. Они ни чего не делают.
* **Пустые клетки (тип < 0):** Чёрные клетки. В них нет ни чего.

**Шаг 4: Смотрим на переменные.**
В самом начале файла есть много разных слов. Эти слова называются переменными. Они как рычажки, которые можно поворачивать, что бы менять поведение жителей. Вот самые важные из них:

*   `startEnergy = 0.5;`  - это сколько энергии получает новая клеточка, когда рождается. 0.5 это половина, 1.0 это максимум.
*   `reproduceEnergyThreshold = 0.8;` -  а это сколько энергии надо накопить, что бы размножиться. 0.8 это значит 80% энергии.
* `energyCostPerTick = 0.001;` - сколько энергии тратится за то что просто живеш. 0.001 это очень мало.
* `reproduceEnergyCost = 0.4;` - Сколько энергии тратится при размножении. 0.4 это 40%.
*   `energyPerPrey  = 0.2;` - Сколько энергии получает хищник, когда съедает жертву. 0.2 это 20%.
* `energyAbsorbed = 0.005;` - Сколько энергии поглощает растение за один ход. 0.005 это мало.
* `energyFromDead = 0.15;`- Сколько энергии получает падальщик за поедание мёртвой клетки. 0.15 это 15%.
* `mutantChance = 0.0005;` - Это шанс, что клеточка превратится в мутанта. 0.0005 это очень редко.

**Шаг 5: Меняем Правила!**

*   **Хотим, чтобы новые клеточки были сильнее?** Измените `startEnergy`, например, на `0.7`.
* **Хотим что бы клетки реже размножались?** Измени `reproduceEnergyThreshold` например на `0.9`.
*   **Хотим, чтобы хищники получали больше энергии?**  Измените `energyPerPrey`, например, на `0.4`.
* **Хотим что бы падальщиков было больше?** Измени `energyFromDead` например на `0.3`.
* **Хотим что бы мутантов было больше?** Измени `mutantChance` например на `0.1`
* **Слишком много растений?** Уменьши `energyAbsorbed`

Просто меняйте значения, на свои, и смотрите, что получается!
**Важно**: Меняйте значения только там где я указал. Ничего пока не трогайте в других местах.

**Шаг 6: Создаём Нового Жителя!**

Это самый интересный шаг! Можно создать своё, уникальное клеточное существо. Вот как это сделать:

1.  **Придумай тип!** Какой тип будет у твоего нового жителя?
2.  **Придумай цвет!** Какого цвета он будет? Посмотри на тех жителей которые уже есть, и придумай свой цвет.
3. **Придумай тип**: Дай ему свой уникальный тип. К примеру 9.
4. **Придумай поведение**: Как он будет себя вести?
    * Что он будет есть?
    * Будет ли он перемещатся?
    * Как он будет размножатся?
    * Будет ли он убивать?
    * Сколько энергии ему нужно для жизни?
    * Какие у него будут особенности?
5. **Добавь его в книгу**: Находим в волшебной книге функцию `getColorByType` и в ней добавляем описание цвета для своего типа.
   ```hlsl
   if(type == 9){ // если тип равен 9
        color = vec4(0.1,0.5,0.8,energy); // задаем цвет 
    }
6. **Добавь поведение**: Находим функцию main и там добавляем его поведение. Там уже много примеров.
   ```hlsl
    if(currentState == 9) { // если наш тип
        //пишем тут поведение для нашей клетки
    }
7. **Ограничения**: Помни, что не все функции можно использовать, и не все переменные ты можешь менять. Смотри в описание к игре, какие есть ограничения.
8. **Дополнительно**: Если тебе надо будет ещё что то добавить в шейдер, ты можешь создать свою переменную в начале книги, там где у нас находятся все переменные. После этого ты сможешь её использовать.
**Шаг 7: Сохраняем и Запускаем!**

1. Когда всё изменил, сохрани файл geminimod.hlsl.
2. Запусти "Игру Жизни" и посмотри, что получилось!

**Шаг 8: Экспериментируем!**

 - Меняй правила, меняй значения, создавай новых жителей. Смотри как меняется твой мир.
 - Не бойся ошибаться.

**Помни:**

Это только начало! У тебя в руках настоящий конструктор миров, и ты можешь создать всё, что только пожелаешь!
Сохраняй свои изменения в файле, и ты сможешь вернуться к ним позже.
Делись своими мирами с друзьями.
Не бойся спрашивать если не понимаешь.
Удачи, юный Волшебник Миров!