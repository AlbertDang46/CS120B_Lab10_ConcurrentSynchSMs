/*	Author: Albert Dang adang018
 *      Partner(s) Name: Min-Hua Wu mwu057
 *	Lab Section: 022
 *	Assignment: Lab #10  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct _Task {
    int state;
    unsigned long period;
    unsigned long elapsedTime;
    int (*TickFct) (int);
} Task;

const unsigned char tasksSize = 3;
Task tasks[3];
const unsigned char tasksPeriod = 1;

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}

void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR() {
    unsigned char i;
    for(i = 0; i < tasksSize; i++) {
        if(tasks[i].elapsedTime >= tasks[i].period) {
            tasks[i].state = tasks[i].TickFct(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += tasksPeriod;
    }
}

ISR(TIMER1_COMPA_vect) {
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0) {
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

unsigned char threeLEDs = 0x04;
unsigned char blinkLED = 0x08;

enum TL_States { TL_Start, TL_TL } TL_State;
int TickFct_ThreeLEDs(int state) {
    switch(state) {
        case TL_Start:
            state = TL_TL;
            break;
        case TL_TL:
            state = TL_TL;
            break;
        default:
            state = TL_Start;
            break;
    }

    switch(state) {
        case TL_Start:
            threeLEDs = 0x04;
            break;
        case TL_TL:
            threeLEDs = (threeLEDs == 0x04) ? 0x01 : (threeLEDs << 1);
            break;
        default:
            threeLEDs = 0x04;
            break;
    }
    
    return state;
}

enum BL_States { BL_Start, BL_BL } BL_State;
int TickFct_BlinkLED(int state) {
    switch(state) {
        case BL_Start:
            state = BL_BL;
            break;
        case BL_BL:
            state = BL_BL;
            break;
        default:
            state = BL_Start;
            break;
    }

    switch(state) {
        case BL_Start:
            blinkLED = 0x08;
            break;
        case BL_BL:
            blinkLED = blinkLED ? 0x00 : 0x08;
            break;
        default:
            blinkLED = 0x08;
            break;
    }

    return state;
}

enum CL_States { CL_Start, CL_CL } CL_State;
int TickFct_CombineLED(int state) {
    switch(state) {
        case CL_Start:
            state = CL_CL;
            break;
        case CL_CL:
            state = CL_CL;
            break;
        default:
            state = CL_Start;
            break;
    }

    switch(state) {
        case CL_Start:
            break;
        case CL_CL:
            PORTB = (threeLEDs | blinkLED);
            break;
        default:
            break;
    }

    return state;
}

int main(void) {
    DDRB = 0xFF; PORTB = 0x00;
    
    unsigned char i = 0;
    tasks[i].state = TL_Start;
    tasks[i].period = 1000;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFct_ThreeLEDs;
    i++;    
    tasks[i].state = BL_Start;
    tasks[i].period = 1000;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFct_BlinkLED;
    i++;
    tasks[i].state = CL_Start;
    tasks[i].period = 1000;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFct_CombineLED;

    TimerSet(tasksPeriod);
    TimerOn();

    while (1) {
//        sleep();
    }
    return 0;
}
