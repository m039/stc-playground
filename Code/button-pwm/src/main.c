#include "fw_hal.h"

#define TICK_MS     20    // main loop period - button is re-read and LED duty updated each tick
#define STEP_SLOW   2     // duty step per tick when the button is released (one phase, fade-in or fade-out, ~ 2.6 s)
#define STEP_FAST   16    // duty step per tick while the button is pressed  (one phase ~ 0.3 s)

/* The CCP2 output on P5.5 of this board (STC8G1K08A port option ..._P31_P32_P33_P55) is inverted w.r.t. CCAP2H -
   measured on the hardware: effective duty = 1 - CCAP2H/256, i.e. full on at CCAP2H=0 and off at CCAP2H=255
   (vendor docs/demos do not document it). `brightness` stays intuitive [0..255] (0 = off); the conversion to
   the compare value is done in this single place: */
#define CCP_VALUE(b)  ((b) == 0 ? 255 : (uint8_t)(256 - (b)))

uint8_t brightness = 0;   // LED duty, 0..255 - breathing triangle wave between these limits
uint8_t rising   = 1;     // direction of the current phase: 1 - fading in, 0 - fading out

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
    // Channel 2: 8-bit PWM mode, no interrupt (effective duty is inverted w.r.t. CCAP2H on this board - see CCP_VALUE)
    PCA_PCA2_SetWorkMode(PCA_WorkMode_PWM_NonInterrupt);
    PCA_PWM2_SetBitWidth(PCA_PWM_BitWidth_8);

    // Initial state - LED off (the "off" duty is CCAP2H=255 because of the inverted output), cycle starts with a smooth fade-in
    PCA_PCA2_SetCompareValue(CCP_VALUE(brightness));

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
        uint8_t step = (P32 == RESET) ? STEP_FAST : STEP_SLOW;   // button pressed - whole cycle runs faster

        if(rising)
        {
            // Compare BEFORE truncation: uint8_t + uint8_t is promoted to int, so a step that would
            // wrap around (e.g. 254+2 -> 0) clamps to full on instead of skipping the fade-out phase
            if(brightness + step >= 255)        // reached full on: turn to the fading-out phase
            {
                brightness = 255;
                rising   = 0;
            }
            else
            {
                brightness += step;             // fade in (no wraparound possible here)
            }
        }
        else if(brightness > step)              // same step as the fade-in - equal speed both ways
        {
            brightness -= step;                 // fade out
        }
        else                                    // faded out completely: turn back to the fading-in phase
        {
            brightness = 0;
            rising   = 1;
        }

        PCA_PCA2_ChangeCompareValue(CCP_VALUE(brightness));   // change duty while PWM is running (CCP_VALUE - inverted output)

        SYS_Delay(TICK_MS);
    }
}