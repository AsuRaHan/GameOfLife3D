.data
; ����� ����� ���������� ���������, ���� ����������
ALIVE equ 1
DEAD  equ 0

.code
extern printf : PROC

asmUpdateGrid PROC
    ; rcx: int* currentState
    ; rdx: int* nextState
    ; r8:  CellParams* cellParams
    ; r9:  GridParams* gridParams

    push rbp
    mov rbp, rsp
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    ; ��������� ��������� GridParams � ��������
    mov r12d, [r9 + 0]  ; isToroidal
    mov r13d, [r9 + 4]  ; GW
    mov r14d, [r9 + 8]  ; GH

    ; ��������� ��������� CellParams � ��������
    mov r15d, [r8 + 0]  ; birth
    mov esi,    [r8 + 4]  ; survivalMin
    mov edi,    [r8 + 8]  ; survivalMax

    ; ������� ���� �� ������ ����� (y)
    xor r10d, r10d      ; y = 0
    outer_loop:
        cmp r10d, r14d  ; y < GH
        jge outer_loop_end

        ; ���������� ���� �� ������ ����� (x)
        xor r11d, r11d  ; x = 0
        inner_loop:
            cmp r11d, r13d  ; x < GW
            jge inner_loop_end

            ; ���������� ������� ������� ������
            mov eax, r10d   ; y
            imul eax, r13d  ; y * GW
            add eax, r11d   ; y * GW + x
            mov rbx, rcx    ; ��������� �� currentState
            lea rbx, [rbx + rax * 4] ; &currentState[y * GW + x]
            mov eax, [rbx]  ; currentState[y * GW + x]

            ; ������� ����� �������
            push rcx
            push rdx
            push r8
            push r9
            push r10
            push r11

            mov ecx, r11d ; x
            mov edx, r10d ; y
            mov r8d, r13d ; GW
            mov r9d, r14d ; GH
            mov edi, r12d ; isToroidal

            call count_alive_neighbors

            pop r11
            pop r10
            pop r9
            pop r8
            pop rdx
            pop rcx

            ; ��������� ������� ����
            cmp eax, r15d   ; ����� ������� == birth?
            je  is_born
            cmp eax, esi    ; ����� ������� < survivalMin?
            jl  is_dead
            cmp eax, edi    ; ����� ������� > survivalMax?
            jg  is_dead
            ; ����� �������� ���� ����
            jmp is_alive

            is_born:        ; ������ ���������
                mov eax, ALIVE
                jmp store_state
            is_dead:        ; ������ �������
                mov eax, DEAD
                jmp store_state
            is_alive:       ; ������ ��������, ��������� �������� ���������
                mov eax, [rbx]
                jmp store_state

            store_state:
            ; ��������� ����� ��������� ������ � nextState
            mov rbx, rdx    ; ��������� �� nextState

            mov esi, r10d   ; y
            imul esi, r13d  ; y * GW
            add esi, r11d   ; y * GW + x

            lea rbx, [rdx + rsi * 4]  ; ���������� rdx (nextState) � esi (������)
            mov [rbx], eax

            ; ��������� � ��������� ������
            inc r11d
            jmp inner_loop

        inner_loop_end:
        inc r10d
        jmp outer_loop

    outer_loop_end:

    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbp
    ret

asmUpdateGrid ENDP

count_alive_neighbors PROC
    ; rcx: x
    ; rdx: y
    ; r8: GW
    ; r9: GH
    ; rdi: isToroidal
    push rbp
    mov rbp, rsp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    xor eax, eax ; �������� ������� ����� �������

    mov r10d, ecx ; x
    mov r11d, edx ; y
    mov r12d, r8d ; GW
    mov r13d, r9d ; GH
    mov r14d, edi ; isToroidal

    ; ������� �������
    mov esi, -1 ; i = -1
    neighbor_loop_i:
        cmp esi, 2 ; i < 2
        jge neighbor_loop_i_end

        mov edi, -1 ; j = -1
        neighbor_loop_j:
            cmp edi, 2 ; j < 2
            jge neighbor_loop_j_end

            ; ���������� ������� ������
            cmp esi, 0
            je skip_current
            cmp edi, 0
            je skip_current

            ; ��������� ���������� ������
            mov eax, r10d ; x
            add eax, edi ; x + j
            mov ebx, r11d ; y
            add ebx, esi ; y + i

            ; ��������� �������
            cmp r14d, 0 ; isToroidal == 0?
            je not_toroidal

            ; ������������ �������
            toroidal_x:
            cmp eax, r12d ; x + j < GW
            jl toroidal_y

            sub eax, r12d ; x + j -= GW

            toroidal_y:
            cmp eax, 0
            jge toroidal_y_end

            add eax, r12d

            toroidal_y_end:

            cmp ebx, r13d ; y + i < GH
            jl check_neighbor

            sub ebx, r13d ; y + i -= GH

            check_neighbor:
            cmp ebx, 0
            jge check_neighbor_end

            add ebx, r13d

            check_neighbor_end:

            jmp get_neighbor_state

            not_toroidal:
            cmp eax, 0
            jl skip_neighbor
            cmp eax, r12d
            jge skip_neighbor
            cmp ebx, 0
            jl skip_neighbor
            cmp ebx, r13d
            jge skip_neighbor

            get_neighbor_state:
            ; ��������� ������ ������� ������
            mov r15d, r11d ; y
            imul r15d, r12d ; y * GW
            add r15d, r10d ; y * GW + x

            mov rbx, rcx    ; ��������� �� currentState
            lea rbx, [rcx + r15d * 4] ; &currentState[y * GW + x]

            cmp dword ptr [rbx],0
			je next_neighbor

            inc eax

            skip_neighbor:

			next_neighbor:
            skip_current:
            inc edi
            jmp neighbor_loop_j

        neighbor_loop_j_end:
        inc esi
        jmp neighbor_loop_i

    neighbor_loop_i_end:

    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    pop rbp
    ret
count_alive_neighbors ENDP

END
