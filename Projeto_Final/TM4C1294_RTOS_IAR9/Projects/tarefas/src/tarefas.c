#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include "stdlib.h" 

osThreadId_t thread1_id, thread2_id, thread3_id, thread4_id;
typedef struct {
   uint8_t led;
   uint32_t time;
} LED_TIME;


void thread_time(void *arg){
 LED_TIME* aux=arg; 
 uint8_t state = 0;
 uint8_t led=aux->led;
 uint32_t time=aux->time;
 uint32_t tick;
  
  while(1){
    tick = osKernelGetTickCount();
    
    state ^= led;
    LEDWrite(led, state);
    
    osDelayUntil(tick + time);
  } // while
} // thread2

void main(void){
  LEDInit(LED2 | LED1 | LED3 | LED4);

  LED_TIME led1 = {.led=LED1, .time= 200};
  LED_TIME led2 = {.led=LED2, .time= 300};
  LED_TIME led3 = {.led=LED3, .time= 500};
  LED_TIME led4 = {.led=LED4, .time= 700};

  osKernelInitialize();

  thread1_id = osThreadNew(thread_time, &led1, NULL);
  thread2_id = osThreadNew(thread_time, &led2, NULL);
  thread3_id = osThreadNew(thread_time, &led3, NULL);
  thread4_id = osThreadNew(thread_time, &led4, NULL);

  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
