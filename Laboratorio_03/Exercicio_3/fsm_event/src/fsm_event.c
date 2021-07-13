#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "driverlib/systick.h"
#include "driverleds.h" // Projects/drivers

// MEF com apenas 2 estados e 1 evento temporal que alterna entre eles
// Seleção por evento

typedef enum {Gray0,Gray1,Gray2,Gray3,Gray4,Gray5,Gray6,Gray7} state_t;

volatile uint8_t Evento = 0;

void SysTick_Handler(void){
  Evento = 1;
} // SysTick_Handler

void main(void){
  static state_t Estado = Gray0; // estado inicial da MEF
  
  LEDInit(LED1);
  LEDInit(LED2);
  LEDInit(LED3);
  SysTickPeriodSet(12000000); // f = 1Hz para clock = 24MHz
  SysTickIntEnable();
  SysTickEnable();

  while(1){
    if(Evento){
      Evento = 0;
      switch(Estado){
        case Gray0:
          LEDOff(LED1);
          LEDOff(LED2);
          LEDOff(LED3);
          Estado = Gray1;
          break;
        case Gray1:
          LEDOff(LED1);
          LEDOff(LED2);
          LEDOn(LED3);
          Estado = Gray2;
          break;
        case Gray2:
          LEDOff(LED1);
          LEDOn(LED2);
          LEDOn(LED3);
          Estado = Gray3;
          break;
        case Gray3:
          LEDOff(LED1);
          LEDOn(LED2);
          LEDOff(LED3);
          Estado = Gray4;
          break;
        case Gray4:
          LEDOn(LED1);
          LEDOn(LED2);
          LEDOff(LED3);
          Estado = Gray5;
          break;
        case Gray5:
          LEDOn(LED1);
          LEDOn(LED2);
          LEDOn(LED3);
          Estado = Gray6;
          break;
        case Gray6:
          LEDOn(LED1);
          LEDOff(LED2);
          LEDOn(LED3);
          Estado = Gray7;
          break;
        case Gray7:
          LEDOn(LED1);
          LEDOff(LED2);
          LEDOff(LED3);
          Estado = Gray0;
          break;

      } // switch
    } // if
  } // while
} // main
