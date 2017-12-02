#include <Checkout.h>
#include "msp.h"
#include "Bluetooth.h"
#include "Buzzer.h"
#include "LockButton.h"
#include "Bluetooth.h"
#include "SystemClock.h"
#include "RGB.h"
#include <stdint.h>


uint32_t systickCounter = 0;        //counts how man 0.5s have passed
uint32_t checkoutTimerTicksVal = 0; //how many ticks the bike is being taken out for
uint32_t overtime = 0;  // how many ticks over the above time the bike is out
uint8_t hours = 0;      // how many hours the bike is out for
uint8_t mins = 0;       // how many mins the bike is out for

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    checkoutTimerTicksVal = checkoutTimerTicks(hours, mins); // calculate time to count to

    //configs
    configure_SystemClock();
    configure_Buzzer();
    configure_LockButton();
    configure_Bluetooth();
    configure_RGB();

    // if time has been set use this
    //startSysTick();

    P1->DIR |= BIT0;
    P5->OUT |= BIT5;
    P6->DIR |= BIT0 | BIT1;
    P6->OUT |= (BIT0 | BIT4 | BIT5);

    while(1){

    }
}
