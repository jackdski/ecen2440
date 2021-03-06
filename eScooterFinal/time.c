/*
  * time.c
  *
  *  Created on: Oct 26, 2017
  *      Author: amabo
  */
 #include "time.h"
 #include "msp.h"
 
 extern uint8_t transmit;
 
 void configure_Systick(){
     //Give SysTick a starting value that correlates to .1 seconds
     SysTick->LOAD = SYSTICK_COUNT;
 
     //Enable the Systick Counter, Enable the Interrupt, Set the Clock
     SysTick->CTRL = BIT0 | BIT1 | BIT2;
 }
 
 void SysTick_Handler (){
     transmit = 1;
 }
 
 //This function sets the DCO clock to run at 12MHz and sources it to many clocks
 void configure_clocks(){
     CS-> KEY = 0x695A; //Unlock module for register access
     CS-> CTL0 = 0;     //Reset tuning parameters
 
     //Setup DCO Clock to run at 12MHz
     CS-> CTL0 = (BIT(23) | CS_CTL0_DCORSEL_3);
 
     //Select ACLO = REFO, SMCLK MCLK = DCO
     CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
     CS->KEY = 0;       //Lock CS module for register access.
 
 }
 
