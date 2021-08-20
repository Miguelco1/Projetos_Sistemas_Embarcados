#include <stdbool.h>
#include <stdio.h>
#include "cmsis_os2.h" // CMSIS-RTOS

// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "system_TM4C1294.h"


osThreadId_t thread1_id, thread2_id;
osMutexId_t uart_id;
osMessageQueueId_t fila_mensagem[4];

typedef struct {                                // object data type
  int8_t char_elev;
  osMessageQueueId_t* fila_mensagem_thread;

} ELEVADOR;

//----------
// UART definitions
extern void UARTStdioIntHandler(void);

void UARTInit(void){
  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

  // Initialize the UART for console I/O.
  UARTStdioConfig(0, 115200, SystemCoreClock);

  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  // Configure GPIO Pins for UART mode.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
} // UARTInit

void UART0_Handler(void){
  UARTStdioIntHandler();
  
} // UART0_Handler
//----------



// osRtxIdleThread
__NO_RETURN void osRtxIdleThread(void *argument){
  (void)argument;
  
  while(1){
    //UARTprintf("Idle thread\n");
    asm("wfi");
  } // while
} // osRtxIdleThread

/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
void thread_elevador(void *argument){
 ELEVADOR* aux=argument; 
 int8_t elevador=aux->char_elev;
 osMessageQueueId_t* fila_m;
 fila_m=aux->fila_mensagem_thread;
 int direcao,andar,tick,aux_numb;
 int count_sub=0,count_desc=0,abrindo=0,fechando=0;
 char aux_a='a';
 int andares_pendentes[16];
 for(int i=0; i <16;i=++)
  andares_pendentes[i]=0;
   
 char msg[16];

 osTimerId_t timer_id = osTimerNew(callback, osTimerOnce,NULL, NULL);

  while(1){
  
      if (osMessageQueueGetCount(*fila_m))
      {
         status = osMessageQueueGet(*fila_m, &msg, NULL, 0U);   // wait for message
          if (status == osOK) {
            if (msg[1]=="E")
            {   
                aux_numb=(int)msg[2]*10 + (int)msg[3]
                if(andares_pendentes[aux_numb]==0)
                {
                  andares_pendentes[aux_numb]=1
                  if(aux_numb>andar)
                  {
                    count_sub+=1;
                  }
                  else if(aux_numb<andar)
                  {
                    count_desc+=1;                    
                  }

                    //acende botao
                }
            }
            else if (msg[1]=="A")
            {
              abrindo=0;
              //Delay de porta aberta
              fechando=0;

            }
            else if (msg[1]=="F")
            {
              fechando=0;
              //Delay de porta aberta
              
            }
            else if (msg[1]=="I")
            {
                aux_numb=(msg[2]-aux_a)
                if(andares_pendentes[aux_numb]==0)
                {
                  andares_pendentes[aux_numb]=1
                  if(aux_numb>andar)
                  {
                    count_sub+=1;
                  }
                  else if(aux_numb<andar)
                  {
                    count_desc+=1;                    
                  }

                    //acende botao
                }
            }

            else//Mensagem de mudanca de andar
            {
                if (andares_pendentes[(int)msg[1]])
                {
                  //envia 
                  
                }
            }
          }
      }

 }// while
} // thread2


/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
void app_main (void *argument) {

  ELEVADOR elevador_esquerda = {.char_elev="e", .fila_mensagem_thread=&fila_mensagem[0]};
  ELEVADOR elevador_centro = {.char_elev="c", .fila_mensagem_thread=&fila_mensagem[1]};
  ELEVADOR elevador_direita = {.char_elev="d", .fila_mensagem_thread=&fila_mensagem[2]};

  tid_elevador_esq = osThreadNew(thread_led, &elevador_esquerda, NULL);
  tid_elevador_cen = osThreadNew(thread_led, &elevador_centro, NULL);
  tid_elevador_dir = osThreadNew(thread_led, &elevador_direita, NULL);

  char aux_receiv[16];
  while(1){

    if(UARTRxBytesAvail())
    {
      osMutexAcquire(uart_id, osWaitForever);
      UARTgets(aux_receiv,sizeof(aux_receiv));
      osMutexRelease(uart_id);
   
   if(aux_receiv[1]=="E")
      {
      
        
      }
      else if(aux_receiv[0]=="e")
      {
      
      }
      else if(aux_receiv[0]=="c")
      {
      
      }
      else if(aux_receiv[0]=="d")
      {
      
      }
    UARTFlushRx(false);
    }

  }
  
}

void main(void){
  UARTInit();
  myKernelInfo();
  myKernelState();
  
 /* if(osKernelGetState() == osKernelInactive)
     osKernelInitialize();

  myKernelState();

  thread1_id = osThreadNew(thread1, NULL, &thread1_attr);
  thread2_id = osThreadNew(thread2, NULL, &thread2_attr);
  uart_id = osMutexNew(NULL);

  if(osKernelGetState() == osKernelReady)
    osKernelStart();
*/
  char infobuf[16];
  while(1)
  {
    if(UARTRxBytesAvail())
    {
      UARTgets(infobuf,sizeof(infobuf));
      UARTprintf(infobuf);
    }
  }
} // main
