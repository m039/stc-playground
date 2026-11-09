#include "fw_hal.h"

volatile uint8_t pressed = 0;   // state of the button - only set from INT0 interrupt

// On STC8G/STC8H family external interrupt INT0 is mapped to pin P3.2 (the button).
// Trigger on both rising and falling edges: press/release are distinguished by P32 level.
INTERRUPT(Button_Routine, EXTI_VectInt0)
{
    if (P32 == RESET)   // falling edge - pressed
    {
        P55 = RESET;    // change LED state immediately, right in the interrupt
        pressed = 1;
    }
    else                // rising edge - released
    {
        pressed = 0;
        P55 = SET;
    }
}

void GPIO_Init(void)
{
    GPIO_P5_SetMode(GPIO_Pin_5,  GPIO_Mode_Output_PP);

    GPIO_P3_SetMode(GPIO_Pin_2, GPIO_Mode_Input_HIP);
    GPIO_SetPullUp(GPIO_Port_3, GPIO_Pin_2, HAL_State_ON);
}

void EXTI_Init(void)
{
    EXTI_Int0_SetTrigByBoth;               // trigger on both rising and falling edges
    EXTI_Int0_SetIntState(HAL_State_ON);   // enable INT0 interrupt (EX0 bit in IE reg.)
    EXTI_Global_SetIntState(HAL_State_ON); // enable global interrupts (EA)
}

int main(void)
{
    GPIO_Init();
    EXTI_Init();

    while(1)
    {
        uint8_t period = pressed ? 100 : 1000;   // fast blink while held, slow otherwise

        P55 = RESET;
        SYS_Delay(period);

        P55 = SET;
        SYS_Delay(period);
    }
}