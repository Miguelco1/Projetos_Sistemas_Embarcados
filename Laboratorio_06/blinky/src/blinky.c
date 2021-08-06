/* -------------------------------------------------------------------------- 
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    BLinky.c
 *      Purpose: RTX example program
 *
 *---------------------------------------------------------------------------*/

#include "driverleds.h" // device drivers
#include "driverbuttons.h"// device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"

typedef struct {                                // object data type
  uint8_t led;
  osThreadId_t* next;
} LED_NEXT;
 

//osTimerId_t timer1_id,timer2_id;
osThreadId_t tid_phaseA;                /* Thread id of thread: phase_a      */
osThreadId_t tid_phaseB;                /* Thread id of thread: phase_b      */
osThreadId_t tid_phaseC;                /* Thread id of thread: phase_c      */
osThreadId_t tid_phaseD;                /* Thread id of thread: phase_d      */
osThreadId_t tid_control;                 /* Thread id of thread: clock        */
osMessageQueueId_t fila_mensagem;

osMutexId_t phases_mut_id;
uint8_t button_1=0;
uint8_t button_2=0;
uint8_t button_pressed,led_ativo=0;
uint8_t state=0;
void GPIOJ_Handler(void){
button_pressed=GPIOIntStatus(GPIO_PORTJ_AHB_BASE,USW1 | USW2);
  if(button_pressed==USW1) // Testa estado do push-button SW1
{
  ButtonIntClear(USW1);
  button_1=1;
}
if(button_pressed==USW2) // Testa estado do push-button SW2
{
  ButtonIntClear(USW2);
  button_2=1;

}

} // GPIOJ_Handler



const osMutexAttr_t Phases_Mutex_attr = {
  "PhasesMutex",                            // human readable mutex name
  osMutexRecursive | osMutexPrioInherit,    // attr_bits
  NULL,                                     // memory for control block   
  0U                                        // size for control block
  };




/*----------------------------------------------------------------------------
 *      Switch LED on
 *---------------------------------------------------------------------------*/
void Switch_On (unsigned char led) {
  osMutexAcquire(phases_mut_id, osWaitForever); // try to acquire mutex
  LEDOn(led);
  osMutexRelease(phases_mut_id);
}

/*----------------------------------------------------------------------------
 *      Switch LED off
 *---------------------------------------------------------------------------*/
void Switch_Off (unsigned char led) {
  osMutexAcquire(phases_mut_id, osWaitForever); // try to acquire mutex
  LEDOff(led);
  osMutexRelease(phases_mut_id);
}

void callback(void *arg){

  if (state==1)
      state=0;

  else
     state=1;

}
 // callback


/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
void thread_led(void *argument){
 LED_NEXT* aux=argument; 
 uint8_t led=aux->led;
 osThreadId_t *prox=aux->next;
 int status,msg;
 osTimerId_t timer_id = osTimerNew(callback, osTimerOnce,NULL, NULL);
 int intensidade=0;
 osThreadFlagsWait(0x0001, osFlagsWaitAny ,osWaitForever);    /* wait for an event flag 0x0001 */
  while(1){
    if(!osTimerIsRunning(timer_id))
    {
          if(intensidade == 0 && !osTimerIsRunning(timer_id))
          {  
            Switch_Off(led);
            state=0;
            osTimerStart(timer_id, 10);
          }
          else if (intensidade >=100 && !osTimerIsRunning(timer_id))
          {
            Switch_On(led);
            state=1;
            osTimerStart(timer_id, 10);

          }
          else{
            if(state==0 && !osTimerIsRunning(timer_id))
            {            
              Switch_On(led);
              osTimerStart(timer_id, 10*intensidade/100);
            }
            else if(state==1 && !osTimerIsRunning(timer_id))           
            {
              Switch_Off(led);
              osTimerStart(timer_id, (10-(10*intensidade/100)));
            }
          }
      }
  
      if (osMessageQueueGetCount(fila_mensagem))
      {
         status = osMessageQueueGet(fila_mensagem, &msg, NULL, 0U);   // wait for message
          if (status == osOK) {
            if (msg==1)
            {
              Switch_Off(led);
              osThreadFlagsSet(*prox, 0x0001);            /* set signal to thread 'thread' */
              osThreadFlagsWait(0x0001, osFlagsWaitAny ,osWaitForever);    /* wait for an event flag 0x0001 */
              state=0;
              
            }
            else if(msg==2)
            {
                if(intensidade>=100)
                {
                  intensidade=0;
                }
                else
                {
                  intensidade+=10;
                }
            }
          }
      }
      osThreadYield();
 }// while
} // thread2



/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
void app_main (void *argument) {

  LED_NEXT led1 = {.led=LED1, .next= &tid_phaseB};
  LED_NEXT led2 = {.led=LED2, .next= &tid_phaseC};
  LED_NEXT led3 = {.led=LED3, .next= &tid_phaseD};
  LED_NEXT led4 = {.led=LED4, .next= &tid_phaseA};

  tid_phaseA = osThreadNew(thread_led, &led1, NULL);
  tid_phaseB = osThreadNew(thread_led, &led2, NULL);
  tid_phaseC = osThreadNew(thread_led, &led3, NULL);
  tid_phaseD = osThreadNew(thread_led, &led4, NULL);
  
  osThreadFlagsSet(tid_phaseA, 0x0001);
  while(1){
    int msg=0;
    if (button_1==1)
    {
      button_1=0;
      msg=1;
      if(osMessageQueueGetSpace(fila_mensagem))
       osMessageQueuePut(fila_mensagem, &msg, 0U, 0U);
      
    }
    
    else if (button_2==1)
    {
      button_2=0;
      msg=2;

      if(osMessageQueueGetSpace(fila_mensagem))
       osMessageQueuePut(fila_mensagem, &msg, 0U, 0U);

    }
    osThreadYield();

  }
  
}

int main (void) {
  // System Initialization
  LEDInit(LED4 | LED3 | LED2 | LED1);
  ButtonInit(USW1 | USW2);
  ButtonIntEnable(USW1 | USW2);

  osKernelInitialize();                 // Initialize CMSIS-RTOS
  tid_control=osThreadNew(app_main, NULL, NULL);    // Create application main thread
  fila_mensagem=osMessageQueueNew(16, sizeof(int),NULL);
  
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    // Start thread execution
  }

  while(1);
}
