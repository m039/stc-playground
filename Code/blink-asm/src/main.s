; STC8G1K17A, IRC 11.0592 MHz — мигание светодиода на P5.5 чистым ассемблером sdas8051.
; Аналог ../blink без C и FwLib_STC8: регистры выставляются напрямую по адресам SFR.
; Функций нет → стека нет: всё в главном цикле ниже.

.module blink_asm

.area HOME (CODE)
.area XSEG (DATA)
.area PSEG (DATA)
.area INTV (ABS)

; Карта регистров STC8G (из fw_reg_base.h FwLib_STC8):
.equ P5, 0xC8          ; порт данных
.equ P5M1, 0xC9       ; режимы пинов: бит M1
.equ P5M0, 0xCA       ; режимы пинов: бит M0

.area CSEG (ABS, CODE)
.org 0x0000
_int_reset:
    ljmp main          ; вектор сброса

.org 0x0100
main:
; GPIO_Init(): P5.5 = push-pull output — выставить бит5 в P5M0 и сбросить его в P5M1 (как blink)
    mov a, P5M0
    orl a, #0x20      ; 0b0010_0000 — маска пина P5.5: M0=1 → push-pull
    mov P5M0, a
    mov a, P5M1
    anl a, #0xDF      ; бит5 = 0 (M1=0)
    mov P5M1, a

loop:
; P55 = RESET — светодиод гаснет
    mov a, P5
    anl a, #0xDF      ; сбросить бит5, остальные пины не трогать
    mov P5, a

; Задержка ~1 с при IRC 11.0592 MHz (одноцикловое ядро STC8G):
; внешний 175 × средний 250 × внутренний (djnz r5: 250 итераций × 1 цикл) ≈ 11,03M циклов ≈ 0,997 с
    mov r7, #175       ; внешний цикл
dly_o:
    mov r6, #250      ; средний цикл
dly_m:
    mov r5, #250      ; внутренний цикл
dly_i:
    djnz r5, dly_i     ; 1 цикл на итерацию
    djnz r6, dly_m
    djnz r7, dly_o

; P55 = SET — светодиод горит
    mov a, P5
    orl a, #0x20      ; выставить бит5 в регистре данных порта
    mov P5, a

; Задержка ~1 с (тот же цикл, что выше)
    mov r7, #175       ; внешний цикл
dly_o2:
    mov r6, #250      ; средний цикл
dly_m2:
    mov r5, #250      ; внутренний цикл
dly_i2:
    djnz r5, dly_i2   ; 1 цикл на итерацию
    djnz r6, dly_m2
    djnz r7, dly_o2

    sjmp loop
