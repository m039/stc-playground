#include "fw_hal.h"

#define TICK_MS     20    // main loop period - button is re-read and LED duty updated each tick
#define STEP_SLOW   2     // duty step per tick when the button is released (full fade-out ~ 2.6 s)
#define STEP_FAST   16    // duty step per tick while the button is pressed  (full fade-out ~ 0.3 s)

uint8_t brightness = 255;   // LED duty, 0..255 - unsigned underflow wraps to full on and restarts the cycle

void GPIO_Init(void)
{
    GPIO_P5_SetMode(GPIO_Pin_5,  GPIO_Mode_Output_PP);   // LED pin, then handed over to PCA output CCP2

    // Button is polled in the main loop (no EXTI needed for a fade effect), internal pull-up as in `button`
    GPIO_P3_SetMode(GPIO_Pin_2, GPIO_Mode_Input_HIP);
    GPIO_SetPullUp(GPIO_Port_3, GPIO_Pin_2, HAL_State_ON);
}

void PCA_Init(void)
{
    // Stop PCA counter before configuring it
    PCA_SetCounterState(HAL_State_OFF);
    // Counter does not run in idle mode (it is restarted manually below)
    PCA_SetStopCounterInIdle(HAL_State_ON);
    // Use SYSCLK as clock source: full 8-bit PWM period = 256 sys cycles ~ 23 us (~43 kHz),
    // so the eye averages duty - no visible flicker
    PCA_SetClockSource(PCA_ClockSource_SysClk);
    // No overflow interrupt needed: duty is changed from the main loop while running
    PCA_EnableCounterOverflowInterrupt(HAL_State_OFF);

    // Only channel 2 (CCP2) drives the LED. Channels 0/1 are switched off, so their pins - and P3.2
    // remapped to CCP0 by the port option below - keep plain GPIO behaviour for the button input.
    PCA_PCA0_SetWorkMode(PCA_WorkMode_None);
    PCA_PCA1_SetWorkMode(PCA_WorkMode_None);
    // Channel 2: 8-bit PWM mode, no interrupt; output duty = CCAP2H / 256
    PCA_PCA2_SetWorkMode(PCA_WorkMode_PWM_NonInterrupt);
    PCA_PWM2_SetBitWidth(PCA_PWM_BitWidth_8);

    // Initial state - LED fully on (the cycle restarts here after underflow)
    PCA_PCA2_SetCompareValue(brightness);

    // STC8G1K08A port option 2: CCP2 -> P5.5 (ECI/CCP0/CCP1 are moved to unused pins)
    PCA_SetPort(PCA_AlterPort_G1K08A_P31_P32_P33_P55);

    // Start PWM - from this moment the pin is driven by hardware, writing P55 has no effect on the LED
    PCA_SetCounterState(HAL_State_ON);
}

int main(void)
{
    GPIO_Init();
    PCA_Init();

    while(1)
    {
        uint8_t step = (P32 == RESET) ? STEP_FAST : STEP_SLOW;   // button pressed - fade out faster

        brightness -= step;                        // unsigned underflow wraps back near full on - cycle restarts
        PCA_PCA2_ChangeCompareValue(brightness);   // change duty while PWM is running

        SYS_Delay(TICK_MS);
    }
}