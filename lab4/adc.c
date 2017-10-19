/*
 *
 * acd.c
 * Author: Jack Danielski and Avery Anderson
 * 10-12-17
 *
 */

#include "adc.h"
#include "lab4.h"

#define PROBLEM6


extern CircBuf_t * TXBuf;
extern uint16_t NADC;
extern uint8_t transmit;

void configure_ADC() {
    // Initialize the shared reference module
    // By default, REFMSTR=1 = REFCTL is used to configure the internal reference
    while(REF_A->CTL0 & REF_A_CTL0_GENBUSY);        //If ref generator busy, WAIT
    REF_A->CTL0 = REF_A_CTL0_VSEL_0 | REF_A_CTL0_ON;//Enable internal 1.2V ref
    REF_A->CTL0 &=  ~REF_A_CTL0_TCOFF;              // Turn on Temperature sensor

    // Configure ADC - Pulse sample mode; ADC14SC trigger
    // ADC ON, temperature sample period > 30us
    ADC14->CTL0 |= ADC14_CTL0_SHT0_5 | ADC14_CTL0_ON | ADC14_CTL0_SHP;
    ADC14->CTL0 &= ~ADC14_CTL0_ENC;
    //conf internal temp sensor channel, set resolution to 14
    ADC14->CTL1 = (ADC14_CTL1_TCMAP | ADC14_CTL1_RES_3);

    // Map temp analog channel to MEM0/MCTL0, set 3.3v ref
    ADC14->MCTL[0] = (ADC14_MCTLN_INCH_22 | ADC14_MCTLN_VRSEL0);
    ADC14->IER0 |= ADC14_IER0_IE0; //Enable MCTL0/MEM0(BIT0) Interrupts

    while(!(REF_A->CTL0 & REF_A_CTL0_GENRDY)); // Wait for ref generator to settle
    ADC14->CTL0 |= ADC14_CTL0_ENC; // Enable Conversions

    //Enable Port one button
    P1->SEL0 &= ~(BIT1 | BIT4);
    P1->SEL1 &= ~(BIT1 | BIT4);
    P1->DIR  &= ~(BIT1 | BIT4);
    P1->REN  |=  (BIT1 | BIT4);
    P1->OUT  |=  (BIT1 | BIT4);
    P1->IES  |=  (BIT1 | BIT4);

    P1->DIR |= BIT0;
    P1->OUT &=  ~BIT0;


    P1->IFG   = 0;
    P1->IE   |=  (BIT1 | BIT4);

    //ADC14->IFGR0 &= ~ADC14_IFGR0_IFG0;
    NVIC_EnableIRQ(ADC14_IRQn); // Enable ADC interrupt in NVIC module
    NVIC_EnableIRQ(PORT1_IRQn); // Enable port 1 buttons
}

void configure_serial_port(){
    //Configure UART pins, set 2-UART pins to UART mode
    P1->SEL0 |=  (BIT2 | BIT3);
    P1->SEL1 &= ~(BIT2 | BIT3);

    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST;     //Put eUSCI in reset
    EUSCI_A0->CTLW0 |= (BIT7);                  //Select Frame parameters and source
    EUSCI_A0->BRW = 78;                         //Set Baud Rate
    EUSCI_A0->MCTLW |= (BIT0 | BIT5);           //Set modulator bits
    EUSCI_A0->CTLW0 &= ~(EUSCI_A_CTLW0_SWRST);  //Initialize eUSCI

#ifndef BLOCKING
    EUSCI_A0->IFG &= ~(BIT1 | BIT0);
    UCA0IE |= (BIT0 | BIT1);  //Turn on interrupts for RX and TX
    NVIC_EnableIRQ(EUSCIA0_IRQn);
#endif
}

void configure_clocks(){
    CS-> KEY = 0x695A; //Unlock module for register access
    CS-> CTL0 = 0;     //Reset tuning parameters
    CS-> CTL0 = (BIT(23) | CS_CTL0_DCORSEL_3);     //Setup DCO Clock

    //Select ACLO = REFO, SMCLK MCLK = DCO
    CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
    CS->KEY = 0;       //Lock CS module for register access.

}

void UART_send_n(uint8_t * data, uint8_t length){
    //Code to iterate through the transmit data
    if(!data)
        return;
    volatile uint8_t n;

    for(n = 0; n<length; n++){
        UART_send_byte(data[n]);
    }
}


void UART_send_byte(uint8_t data){
    EUSCI_A0->TXBUF = data;
}

void EUSCIA0_IRQHandler(){
    if (EUSCI_A0->IFG & BIT1){
        //Transmit Stuff
        if(isEmpty(TXBuf)){
            EUSCI_A0->IFG &= ~BIT1;
            ADC14->CTL0 &= ~(ADC14_CTL0_ENC);
            ADC14->CTL0 |= ADC14_CTL0_ON ; // Disable Conversions
            ADC14->CTL0 |= (ADC14_CTL0_ENC);
            return;
        }
        UART_send_byte(removeItem(TXBuf));
    }

}

void ADC14_IRQHandler(){

    if(ADC14_IFGR0_IFG0){
        P1->DIR |= BIT0;
        //P1->OUT ^= BIT0;
        NADC = ADC14->MEM[0];
        uint8_t CString[7];

        itoa(ADC14->MEM[0], 5, CString);
        CString[6] = 0x0D;
        CString[5] = 0x0A;
        //addItemCircBuf(TXBuf, ADC14->MEM[0]);
        loadToBuf(TXBuf,CString,5);

    }
}
void PORT1_IRQHandler(){
#ifndef PROBLEM6
    if(P1->IFG & BIT1 || P1->IFG & BIT4){
        P1->OUT ^= BIT0;
        printTemps();
    }
#endif
#ifdef PROBLEM6
    if (P1->IFG & BIT1 || P1->IFG & BIT4) {
        transmit = 1;
    }
#endif
    P1->IFG = 0;
}

void printTemps(){
    float Ctemp = 0;
    uint8_t CString[7];

    uint8_t KString[9];

    uint8_t FString[9];

    Ctemp = (0.0381)*(float)(NADC)-360.5;
    ftoa(Ctemp,2,5,CString);
    CString[6] = 0x0D;
    CString[5] = 0x0A;

    float Ftemp = (Ctemp*1.8)+32.0;

    ftoa((Ctemp+273),2,7,KString);
    KString[8] = 0x0D;
    KString[7] = 0x0A;

    ftoa(Ftemp,2,7,FString);
    FString[8] = 0x0D;
    FString[7] = 0x0A;

    loadToBuf(TXBuf,"Temps in C, K and F",19);
    addItemCircBuf(TXBuf, 0x0A);
    addItemCircBuf(TXBuf, 0x0D);

    loadToBuf(TXBuf,CString,7);
    loadToBuf(TXBuf,KString,9);
    loadToBuf(TXBuf,FString,9);

    if(!isEmpty(TXBuf)){
        EUSCI_A0->IFG |= BIT1;
    }
}

void problemSix() {
    EUSCI_A0->IFG |= BIT1;
    while(!isEmpty(TXBuf));
    resetCircBuf(TXBuf);
    P1->OUT &= ~BIT0;
}