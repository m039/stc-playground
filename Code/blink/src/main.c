#include "fw_hal.h"

void GPIO_Init(void)
{
    GPIO_P5_SetMode(GPIO_Pin_5,  GPIO_Mode_Output_PP);
}

int main(void)
{
    GPIO_Init();

    while(1)
    {
        P55 = RESET;

        SYS_Delay(100);

        P55 = SET;

        SYS_Delay(1000);
    }
}