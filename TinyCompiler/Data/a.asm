
assume cs:code,ss:stack,ds:data
data segment
  
  t1 db 1 dup(0)
 t2 db 1 dup(0)
 t3 db 1 dup(0)
 fact db 1 dup(0)
 x db 1  dup(0)
 t4 db 1 dup(0)
 t5 db 1 dup(0)
 
data ends
stack segment
  s dw 10 dup(0)
stack ends
code segment
start:

;show char segment
     mov ax,0200h;set once and keep this function


    mov ah,01H
    int 21H
    mov bl,al
    sub bl,30h
    mov x,bl

    mov al,0
    mov bl,x
   cmp al,bl
    jb lt0
    mov t1,0
    jmp lt_over0
    lt0:mov t1,1
    lt_over0:

    cmp t1,1
    je L1

    L2:

    mov bl,x
    sub bl,1
    mov t2,bl

    mov al,t2
    mov fact,al
    mov ax,fact
    mov bx,x
    mul bx
    mov t3,al

    mov al,t3
    mov fact,al
    mov bl,x
    sub bl,1
    mov t4,bl

    mov al,t4
    mov x,al
    mov al,x
    mov bl,0
   cmp al,bl
    je eq0
    mov t5,0
    jmp eq_over0
    eq0:mov t5,1
    eq_over0:

    cmp t5,1
    je L2

    mov bl,fact
    add bl,30h
    mov dl,bl
    mov ax,0200h
    int 21h

    L1:

    mov ah,4ch
    int 21h

code ends
end start
