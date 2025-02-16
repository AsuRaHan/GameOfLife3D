; Файл: asmPaintCell.asm

.code

; ============================================================================================================================

; Процедура asmUpdateGrid
; RCX  = int* currentState
; RDX  = int* nextState
; R8   = int GW
; R9   = int GH
; [RSP + 40] = int isToroidal
; [RSP + 48] = int birth
; [RSP + 56] = int survivalMin
; [RSP + 64] = int survivalMax
; [RSP + 72] = int overpopulation
asmUpdateGrid proc
    push rbp
    mov rbp, rsp        ; Сохраняем исходное значение rsp в rbp
    sub rsp, 96         ; Выделяем место: 56 байт для регистров + 32 байта для локальных переменных + 8 байт для выравнивания
    and rsp, -16        ; Выравнивание стека по 16 байтам

    ; Сохраняем регистры вручную
    mov [rbp - 8], rbx
    mov [rbp - 16], rsi
    mov [rbp - 24], rdi
    mov [rbp - 32], r12
    mov [rbp - 40], r13
    mov [rbp - 48], r14
    mov [rbp - 56], r15

    ; Сохраняем параметры в регистрах
    mov r12, rcx        ; r12 = currentState
    mov r13, rdx        ; r13 = nextState
    mov r14, r8         ; r14 = GW (64-bit)
    mov r15, r9         ; r15 = GH (64-bit)
    mov ebx, [rbp + 16 + 40] ; ebx = isToroidal
    mov r10d, [rbp + 16 + 48] ; r10d = birth
    mov r11d, [rbp + 16 + 56] ; r11d = survivalMin
    mov r8d, [rbp + 16 + 64]  ; r8d = survivalMax
    mov r9d, [rbp + 16 + 72]  ; r9d = overpopulation (не используется, синхронизировано с GPU)

    ; Инициализируем счетчики циклов
    xor rsi, rsi        ; rsi = y = 0
outer_loop:
    cmp rsi, r15        ; Сравниваем y с GH
    jge end_outer_loop  ; Если y >= GH, выходим

    xor rdi, rdi        ; rdi = x = 0
inner_loop:
    cmp rdi, r14        ; Сравниваем x с GW
    jge end_inner_loop  ; Если x >= GW, выходим

    ; Подсчитываем количество живых соседей
    xor eax, eax        ; eax = count (количество живых соседей)

    ; Смещения для 8 соседей: {-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}
    mov edx, -1         ; edx = dy
dy_loop:
    cmp edx, 1
    jg end_dy_loop

    mov ecx, -1         ; ecx = dx
dx_loop:
    cmp ecx, 1
    jg end_dx_loop

    ; Пропускаем центр (dx=0, dy=0)
    test ecx, ecx
    jnz check_neighbor
    test edx, edx
    jz next_neighbor

check_neighbor:
    ; Вычисляем координаты соседа: nx = x + dx, ny = y + dy
    mov rbx, rdi        ; rbx = x
    add rbx, rcx        ; rbx = nx = x + dx
    mov r11, rsi        ; r11 = y
    add r11, rdx        ; r11 = ny = y + dy

    ; Проверяем тороидальность
    test ebx, ebx       ; isToroidal?
    jz non_toroidal

    ; Тороидальная коррекция: nx = (nx + GW) % GW
    mov rax, rbx        ; rax = nx
    cqo                 ; Подготовка к делению
    idiv r14            ; Делим nx на GW, остаток в rdx
    mov rbx, rdx        ; rbx = nx % GW
    test rbx, rbx       ; Если nx < 0
    jge no_adjust_x
    add rbx, r14        ; nx = nx + GW
no_adjust_x:

    ; ny = (ny + GH) % GH
    mov rax, r11        ; rax = ny
    cqo                 ; Подготовка к делению
    idiv r15            ; Делим ny на GH, остаток в rdx
    mov r11, rdx        ; r11 = ny % GH
    test r11, r11       ; Если ny < 0
    jge no_adjust_y
    add r11, r15        ; ny = ny + GH
no_adjust_y:
    jmp check_bounds

non_toroidal:
    ; Проверяем границы для нетороидального мира
    cmp rbx, 0
    jl next_neighbor
    cmp rbx, r14
    jge next_neighbor
    cmp r11, 0
    jl next_neighbor
    cmp r11, r15
    jge next_neighbor

check_bounds:
    ; Вычисляем индекс соседа: index = ny * GW + nx
    mov rax, r11        ; rax = ny
    imul rax, r14       ; rax = ny * GW
    add rax, rbx        ; rax = ny * GW + nx

    ; Дополнительная проверка границ массива
    cmp rax, 0
    jl skip_neighbor
    mov rbx, r14        ; rbx = GW
    imul rbx, r15       ; rbx = GW * GH
    cmp rax, rbx
    jge skip_neighbor

    ; Читаем состояние соседа
    mov ebx, [r12 + rax * 4]  ; ebx = currentState[ny * GW + nx]
    test ebx, ebx
    jz skip_neighbor
    inc eax             ; Увеличиваем счетчик живых соседей

skip_neighbor:
next_neighbor:
    inc ecx             ; dx++
    jmp dx_loop

end_dx_loop:
    inc edx             ; dy++
    jmp dy_loop

end_dy_loop:
    ; Сохраняем количество соседей
    mov rbx, rax        ; rbx = count (количество живых соседей)

    ; Получаем текущее состояние клетки
    mov rax, rsi        ; rax = y
    imul rax, r14       ; rax = y * GW
    add rax, rdi        ; rax = y * GW + x
    mov eax, [r12 + rax * 4]  ; eax = currentState[y * GW + x]
    test eax, eax
    setnz al            ; al = currentState != 0 (1 если жива, 0 если мертва)
    movzx eax, al       ; eax = currentState (0 или 1)

    ; Применяем правила "Игры Жизни"
    test eax, eax
    jz dead_cell

    ; Живая клетка: проверяем выживание
    cmp ebx, r11d       ; Сравниваем count с survivalMin
    jl dead             ; Если меньше survivalMin, клетка умирает
    cmp ebx, r8d        ; Сравниваем count с survivalMax
    jg dead             ; Если больше survivalMax, клетка умирает
    mov edx, 1          ; Клетка выживает
    jmp apply_state

dead:
    mov edx, 0          ; Клетка умирает
    jmp apply_state

dead_cell:
    ; Мертвая клетка: проверяем рождение
    cmp ebx, r10d       ; Сравниваем count с birth
    jne no_birth
    mov edx, 1          ; Клетка оживает
    jmp apply_state

no_birth:
    mov edx, 0          ; Клетка остается мертвой

apply_state:
    ; Сохраняем новое состояние в nextState
    mov rax, rsi        ; rax = y
    imul rax, r14       ; rax = y * GW
    add rax, rdi        ; rax = y * GW + x
    mov [r13 + rax * 4], edx  ; nextState[y * GW + x] = edx

next_iteration:
    inc rdi             ; x++
    jmp inner_loop

end_inner_loop:
    inc rsi             ; y++
    jmp outer_loop

end_outer_loop:
    ; Восстанавливаем регистры
    mov rbx, [rbp - 8]
    mov rsi, [rbp - 16]
    mov rdi, [rbp - 24]
    mov r12, [rbp - 32]
    mov r13, [rbp - 40]
    mov r14, [rbp - 48]
    mov r15, [rbp - 56]

    mov rsp, rbp        ; Восстанавливаем rsp
    pop rbp             ; Восстанавливаем rbp
    ret

asmUpdateGrid endp
; ============================================================================================================================

end