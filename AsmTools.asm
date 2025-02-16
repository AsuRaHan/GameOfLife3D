; ����: asmPaintCell.asm

.code

; ============================================================================================================================

; ��������� asmUpdateGrid
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
    mov rbp, rsp        ; ��������� �������� �������� rsp � rbp
    sub rsp, 96         ; �������� �����: 56 ���� ��� ��������� + 32 ����� ��� ��������� ���������� + 8 ���� ��� ������������
    and rsp, -16        ; ������������ ����� �� 16 ������

    ; ��������� �������� �������
    mov [rbp - 8], rbx
    mov [rbp - 16], rsi
    mov [rbp - 24], rdi
    mov [rbp - 32], r12
    mov [rbp - 40], r13
    mov [rbp - 48], r14
    mov [rbp - 56], r15

    ; ��������� ��������� � ���������
    mov r12, rcx        ; r12 = currentState
    mov r13, rdx        ; r13 = nextState
    mov r14, r8         ; r14 = GW (64-bit)
    mov r15, r9         ; r15 = GH (64-bit)
    mov ebx, [rbp + 16 + 40] ; ebx = isToroidal
    mov r10d, [rbp + 16 + 48] ; r10d = birth
    mov r11d, [rbp + 16 + 56] ; r11d = survivalMin
    mov r8d, [rbp + 16 + 64]  ; r8d = survivalMax
    mov r9d, [rbp + 16 + 72]  ; r9d = overpopulation (�� ������������, ���������������� � GPU)

    ; �������������� �������� ������
    xor rsi, rsi        ; rsi = y = 0
outer_loop:
    cmp rsi, r15        ; ���������� y � GH
    jge end_outer_loop  ; ���� y >= GH, �������

    xor rdi, rdi        ; rdi = x = 0
inner_loop:
    cmp rdi, r14        ; ���������� x � GW
    jge end_inner_loop  ; ���� x >= GW, �������

    ; ������������ ���������� ����� �������
    xor eax, eax        ; eax = count (���������� ����� �������)

    ; �������� ��� 8 �������: {-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}
    mov edx, -1         ; edx = dy
dy_loop:
    cmp edx, 1
    jg end_dy_loop

    mov ecx, -1         ; ecx = dx
dx_loop:
    cmp ecx, 1
    jg end_dx_loop

    ; ���������� ����� (dx=0, dy=0)
    test ecx, ecx
    jnz check_neighbor
    test edx, edx
    jz next_neighbor

check_neighbor:
    ; ��������� ���������� ������: nx = x + dx, ny = y + dy
    mov rbx, rdi        ; rbx = x
    add rbx, rcx        ; rbx = nx = x + dx
    mov r11, rsi        ; r11 = y
    add r11, rdx        ; r11 = ny = y + dy

    ; ��������� ��������������
    test ebx, ebx       ; isToroidal?
    jz non_toroidal

    ; ������������ ���������: nx = (nx + GW) % GW
    mov rax, rbx        ; rax = nx
    cqo                 ; ���������� � �������
    idiv r14            ; ����� nx �� GW, ������� � rdx
    mov rbx, rdx        ; rbx = nx % GW
    test rbx, rbx       ; ���� nx < 0
    jge no_adjust_x
    add rbx, r14        ; nx = nx + GW
no_adjust_x:

    ; ny = (ny + GH) % GH
    mov rax, r11        ; rax = ny
    cqo                 ; ���������� � �������
    idiv r15            ; ����� ny �� GH, ������� � rdx
    mov r11, rdx        ; r11 = ny % GH
    test r11, r11       ; ���� ny < 0
    jge no_adjust_y
    add r11, r15        ; ny = ny + GH
no_adjust_y:
    jmp check_bounds

non_toroidal:
    ; ��������� ������� ��� ��������������� ����
    cmp rbx, 0
    jl next_neighbor
    cmp rbx, r14
    jge next_neighbor
    cmp r11, 0
    jl next_neighbor
    cmp r11, r15
    jge next_neighbor

check_bounds:
    ; ��������� ������ ������: index = ny * GW + nx
    mov rax, r11        ; rax = ny
    imul rax, r14       ; rax = ny * GW
    add rax, rbx        ; rax = ny * GW + nx

    ; �������������� �������� ������ �������
    cmp rax, 0
    jl skip_neighbor
    mov rbx, r14        ; rbx = GW
    imul rbx, r15       ; rbx = GW * GH
    cmp rax, rbx
    jge skip_neighbor

    ; ������ ��������� ������
    mov ebx, [r12 + rax * 4]  ; ebx = currentState[ny * GW + nx]
    test ebx, ebx
    jz skip_neighbor
    inc eax             ; ����������� ������� ����� �������

skip_neighbor:
next_neighbor:
    inc ecx             ; dx++
    jmp dx_loop

end_dx_loop:
    inc edx             ; dy++
    jmp dy_loop

end_dy_loop:
    ; ��������� ���������� �������
    mov rbx, rax        ; rbx = count (���������� ����� �������)

    ; �������� ������� ��������� ������
    mov rax, rsi        ; rax = y
    imul rax, r14       ; rax = y * GW
    add rax, rdi        ; rax = y * GW + x
    mov eax, [r12 + rax * 4]  ; eax = currentState[y * GW + x]
    test eax, eax
    setnz al            ; al = currentState != 0 (1 ���� ����, 0 ���� ������)
    movzx eax, al       ; eax = currentState (0 ��� 1)

    ; ��������� ������� "���� �����"
    test eax, eax
    jz dead_cell

    ; ����� ������: ��������� ���������
    cmp ebx, r11d       ; ���������� count � survivalMin
    jl dead             ; ���� ������ survivalMin, ������ �������
    cmp ebx, r8d        ; ���������� count � survivalMax
    jg dead             ; ���� ������ survivalMax, ������ �������
    mov edx, 1          ; ������ ��������
    jmp apply_state

dead:
    mov edx, 0          ; ������ �������
    jmp apply_state

dead_cell:
    ; ������� ������: ��������� ��������
    cmp ebx, r10d       ; ���������� count � birth
    jne no_birth
    mov edx, 1          ; ������ �������
    jmp apply_state

no_birth:
    mov edx, 0          ; ������ �������� �������

apply_state:
    ; ��������� ����� ��������� � nextState
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
    ; ��������������� ��������
    mov rbx, [rbp - 8]
    mov rsi, [rbp - 16]
    mov rdi, [rbp - 24]
    mov r12, [rbp - 32]
    mov r13, [rbp - 40]
    mov r14, [rbp - 48]
    mov r15, [rbp - 56]

    mov rsp, rbp        ; ��������������� rsp
    pop rbp             ; ��������������� rbp
    ret

asmUpdateGrid endp
; ============================================================================================================================

end