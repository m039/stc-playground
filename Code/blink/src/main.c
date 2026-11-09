#include <compiler.h>

/* Ручное описание регистров STC8G для компилятора SDCC */
__sfr __at (0xC8) P5;     // Регистр данных Порта 5
__sbit __at (0xCD) P5_5;   // Бит 5 в Порту 5 (адрес вычисляется как 0xC8 + 5)

// Регистры настройки режима портов для P5 (Port 5 Mode Registers)
__sfr __at (0xC9) P5M1;
__sfr __at (0xCA) P5M0;

// Простая функция задержки
void delay_ms(unsigned long ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 114; j++) {
            __asm__("nop"); // Пустая инструкция
        }
    }
}

void main(void) {
    /* Настройка P5.5 в режим Push-Pull (сильный выход) 
       Для Push-Pull: P5M1.5 = 0, P5M0.5 = 1 */
    P5M1 &= ~(1 << 5); 
    P5M0 |=  (1 << 5);

    while (1) {
        P5_5 = 0; // Включаем (если светодиод подключен катодом к пину)
        delay_ms(5000);
        P5_5 = 1; // Выключаем
        delay_ms(5000);
    }
}