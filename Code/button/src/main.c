#include "fw_hal.h"

void GPIO_Init(void)
{
    GPIO_P5_SetMode(GPIO_Pin_5,  GPIO_Mode_Output_PP);

    GPIO_P3_SetMode(GPIO_Pin_2, GPIO_Mode_Input_HIP);
    GPIO_SetPullUp(GPIO_Port_3, GPIO_Pin_2, HAL_State_ON);
}

int main(void)
{
    GPIO_Init();

    while(1)
    {
        uint8_t pressed = P32 == RESET;

        P55 = RESET;

        SYS_Delay(pressed? 100 : 1000);

        P55 = SET;

        SYS_Delay(pressed? 100 : 1000);
    }
}