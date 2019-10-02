/*
 * Toggling LEDs using special function registers by their names
 *  defined in the TivaWare header file
 *
 *
 */

#include <stdint.h>
#include "inc/tm4c123gh6pm.h"

void delayMs(int n);

int main(void)
{
    /* enable clock to GPIOF at clock gating control register */
    SYSCTL_RCGC2_R |= 0x00000020;
    /* enable the GPIO pins for the LED (PF3, 2 1) as output */
    GPIO_PORTF_DIR_R = 0x0E;

    /* enable the GPIO pins for digital function */
    GPIO_PORTF_DEN_R = 0x0E;

    while(1) {
        GPIO_PORTF_DATA_R = 0x0E; /* turn on all LEDs */
        delayMs(500);
        GPIO_PORTF_DATA_R = 0;     /* turn off all LEDs */
        delayMs(500);
    }
}
/* delay n milliseconds (16 MHz CPU clock) */
void delayMs(int n)
{
    int i, j;
    for(i = 0 ; i < n; i++)
        for(j = 0; j < 3180; j++){} /* do nothing for 1 ms */
}
