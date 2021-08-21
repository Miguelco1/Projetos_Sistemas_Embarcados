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


osThreadId_t tid_elevador_esq, tid_elevador_cen,tid_elevador_dir,main_thread;
osMutexId_t uart_id;
osMessageQueueId_t fila_mensagem_esq;
osMessageQueueId_t fila_mensagem_cen;
osMessageQueueId_t fila_mensagem_dir;
enum estado_elevador {PARADO,ESPERANDO,MOVIMENTO,SOLICITA_MOVIMENTO};
enum dir{SUBINDO,DESCENDO,PARADO_DIR};
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
  //osMutexAcquire(uart_id, 0);
  UARTStdioIntHandler();
 /* char msg_aux[16];
    if(UARTRxBytesAvail())
    {
      UARTgets(msg_aux,sizeof(msg_aux));
      UARTprintf(msg_aux);
      if(osMessageQueueGetSpace(fila_msgs_recebidas)
        osMessageQueuePut(fila_msgs_recebidas, &msg_aux, 0U, 0U);

    }
 osMutexRelease(uart_id);
*/

} // UART0_Handler
//----------



/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
void thread_elevador(void *argument){
 ELEVADOR* aux=argument; 
 char elevador=aux->char_elev;
 osMessageQueueId_t* fila_m;
 fila_m=aux->fila_mensagem_thread;
 int direcao,aux_numb,status,estado_elevador;
 
 int count_sub=0,count_desc=0,andar=0;
 char botao;
 char aux_a='a';
 char aux_0='0';
 int aux_ext;
 int andares_pendentes[16];
 estado_elevador=PARADO;
 direcao=PARADO_DIR;
 
 for(int i=0; i <16;i++)
  andares_pendentes[i]=0;
   
 char msg[16];
  osMutexAcquire(uart_id, osWaitForever);
  UARTprintf("%cr\r",elevador);  //acende botao
  UARTFlushTx(true);
  osMutexRelease(uart_id); 
 
 
  while(1){
  
      if (osMessageQueueGetCount(*fila_m))
      {
         status = osMessageQueueGet(*fila_m, &msg, NULL, 0U);   // wait for message
          if (status == osOK) {
            if (msg[1]=='E')
            {   
                if(msg[4]=='s')                  
                  aux_ext=2;
                if(msg[4]=='d')                  
                  aux_ext=3;
                aux_numb=(int)(msg[2]-aux_0)*10 + (int)(msg[3]-aux_0);
                if(andares_pendentes[aux_numb]==0)
                {
                  andares_pendentes[aux_numb]=aux_ext;
                  if(aux_numb>andar)
                  {
                  count_sub+=1;

                  }
                  else if(aux_numb<andar)
                  {
                  count_desc+=1;                    
                    
                  }
                  else if(aux_numb==andar && estado_elevador==PARADO)
                  {
                    osMutexAcquire(uart_id, osWaitForever);
                    UARTprintf("%ca\r",elevador);  //abre a porta               
                    osMutexRelease(uart_id);
                    estado_elevador=ESPERANDO;                 
                  }
                  if(estado_elevador==PARADO)                
                    estado_elevador=SOLICITA_MOVIMENTO;
                  


                }
            }
            
            else if (msg[1]=='I')
            {
                aux_numb=(msg[2]-aux_a);
                if (andares_pendentes[aux_numb]==2 || andares_pendentes[aux_numb]==3)
                {
                  andares_pendentes[aux_numb]=1;
                  osMutexAcquire(uart_id, osWaitForever);
                  botao=aux_a+aux_numb;
                  UARTprintf("%cL%c\r",elevador,botao);  //acende botao
                  osMutexRelease(uart_id);
                }
                  if(andares_pendentes[aux_numb]==0 )
                {
                  if(aux_numb>andar)
                  {
                  botao=aux_a+aux_numb;
                  osMutexAcquire(uart_id, osWaitForever);
                  UARTprintf("%cL%c\r",elevador,botao);  //acende botao
                  osMutexRelease(uart_id);
                  count_sub+=1;
                  andares_pendentes[aux_numb]=1;
                  }
                  else if(aux_numb<andar)
                  {
                  botao=aux_a+aux_numb;
                  osMutexAcquire(uart_id, osWaitForever);
                  UARTprintf("%cL%c\r",elevador,botao);  //acende botao
                  osMutexRelease(uart_id);
                  count_desc+=1;                    
                  andares_pendentes[aux_numb]=1;
                  }
                  else if(aux_numb==andar && estado_elevador==PARADO)
                  {
                    osMutexAcquire(uart_id, osWaitForever);
                    UARTprintf("%ca\r",elevador);  //abre a porta               
                    osMutexRelease(uart_id);
                    estado_elevador=ESPERANDO;                
                  }                  
                  if(estado_elevador==PARADO)
                      estado_elevador=SOLICITA_MOVIMENTO;
                  

                }
              
            }
            
            else if (msg[1]=='A')
            {
              botao=aux_a+andar;
              osMutexAcquire(uart_id, osWaitForever);
              UARTprintf("%cD%c\r",elevador,botao);  //Desliga botao
              osMutexRelease(uart_id);
              osDelay(2000);
              //Delay de porta aberta
              osMutexAcquire(uart_id, osWaitForever);
              UARTprintf("%cf\r",elevador);  //Fecha a porta               
              osMutexRelease(uart_id);

            }
            else if (msg[1]=='F')
            {
              estado_elevador= SOLICITA_MOVIMENTO;
            }

            else//Mensagem de mudanca de andar
            {
                if(msg[2]=='\0')
                aux_numb=(int)(msg[1]-aux_0);
                else
                aux_numb=(int)(msg[1]-aux_0)*10 + (int)(msg[2]-aux_0);
                    if (direcao==DESCENDO)
                     andar--;
                    if (direcao==SUBINDO)
                     andar++;
                if ((andares_pendentes[aux_numb]==2 && direcao==SUBINDO) || (andares_pendentes[aux_numb]==3 && direcao==DESCENDO)  || andares_pendentes[aux_numb]==1)
                {
                  andares_pendentes[aux_numb]=0;
                    if (direcao==DESCENDO)
                     count_desc--;
                    if (direcao==SUBINDO)
                     count_sub--;
                    osMutexAcquire(uart_id, osWaitForever);
                    UARTprintf("%cp\r",elevador);  //Para o elevador                
                    osMutexRelease(uart_id);
                    osDelay(100);
                    osMutexAcquire(uart_id, osWaitForever);
                    UARTprintf("%ca\r",elevador);  //abre a porta               
                    osMutexRelease(uart_id);
                    estado_elevador=ESPERANDO;
                    //envia 
                  
                }
                else if((andares_pendentes[aux_numb]==2 && direcao==DESCENDO && count_sub==0) || (andares_pendentes[aux_numb]==3 && direcao==SUBINDO && count_desc==0) )
                {
                  andares_pendentes[aux_numb]=0;
                    if (direcao==DESCENDO)
                     count_desc--;
                    if (direcao==SUBINDO)
                     count_sub--;
                    osMutexAcquire(uart_id, osWaitForever);
                   if (andar!=0)
                    UARTprintf("%cp\r",elevador);  //Para o elevador                
                    osMutexRelease(uart_id);
                    osDelay(100);
                    osMutexAcquire(uart_id, osWaitForever);
                    UARTprintf("%ca\r",elevador);  //abre a porta               
                    osMutexRelease(uart_id);
                    estado_elevador=ESPERANDO;

                }    
                else if((andares_pendentes[aux_numb]==2 && direcao==DESCENDO))
                {
                  count_sub++;
                  count_desc--;

                }
                else if((andares_pendentes[aux_numb]==3 && direcao==SUBINDO))
                {
                  count_desc++;
                  count_sub--;
        

                }                
            }
          }
      }
      if((count_desc>0 || count_sub>0 )&& estado_elevador==SOLICITA_MOVIMENTO)
      {
        if(direcao==SUBINDO && count_sub>0)
        {
          direcao=SUBINDO;
         osMutexAcquire(uart_id, osWaitForever);
         UARTprintf("%cs\r",elevador);  //Sobe               
         osMutexRelease(uart_id);
        }
        else if(direcao==DESCENDO && count_desc>0)
        {
         direcao=DESCENDO;
         osMutexAcquire(uart_id, osWaitForever);
         UARTprintf("%cd\r",elevador);  //Desce               
         osMutexRelease(uart_id);
          
        }
        else if(direcao==SUBINDO && count_desc>0)
        {
         direcao=DESCENDO;
         osMutexAcquire(uart_id, osWaitForever);
         UARTprintf("%cd\r",elevador);  //Desce               
         osMutexRelease(uart_id);
          
        }
        else if(direcao==DESCENDO && count_sub>0)
        {
           direcao=SUBINDO;
          osMutexAcquire(uart_id, osWaitForever);
          UARTprintf("%cs\r",elevador);  //Sobe               
          osMutexRelease(uart_id);
            
        }
        else if(direcao==PARADO_DIR && count_sub>0)
        {
         direcao=SUBINDO;
        osMutexAcquire(uart_id, osWaitForever);
        UARTprintf("%cs\r",elevador);  //Sobe               
        osMutexRelease(uart_id);
            
        }        
        else if(direcao==PARADO_DIR && count_desc>0)
        {
         direcao=DESCENDO;
        osMutexAcquire(uart_id, osWaitForever);
        UARTprintf("%cd\r",elevador);  //Desce               
        osMutexRelease(uart_id);
            
        }        

        estado_elevador=MOVIMENTO;
        }
        else if((estado_elevador==ESPERANDO || estado_elevador==PARADO )  && count_desc==0 && count_sub==0 )
        {
         direcao=PARADO_DIR;
         estado_elevador=PARADO;
        
        }
          
      
            //osDelay(10);
        
      

 }// while
} // thread2


/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
void app_main (void *argument) {
 
  ELEVADOR elevador_esquerda = {.char_elev='e', .fila_mensagem_thread=&fila_mensagem_esq};
  ELEVADOR elevador_centro = {.char_elev='c', .fila_mensagem_thread=&fila_mensagem_cen};
  ELEVADOR elevador_direita = {.char_elev='d', .fila_mensagem_thread=&fila_mensagem_dir};
  
  tid_elevador_esq = osThreadNew(thread_elevador, &elevador_esquerda, NULL);
  tid_elevador_cen = osThreadNew(thread_elevador, &elevador_centro, NULL);
  tid_elevador_dir = osThreadNew(thread_elevador, &elevador_direita, NULL);
  char aux_receiv[16];
  while(1){

    if(UARTRxBytesAvail())
    {
      osMutexAcquire(uart_id, osWaitForever);
      UARTgets(aux_receiv,sizeof(aux_receiv));
      UARTFlushRx();
      osMutexRelease(uart_id);

      if(aux_receiv[0]=='e')
      {
         if(osMessageQueueGetSpace(fila_mensagem_esq))
       osMessageQueuePut(fila_mensagem_esq, &aux_receiv, 0U, 0U);   
      }
      else if(aux_receiv[0]=='c')
      {
         if(osMessageQueueGetSpace(fila_mensagem_cen))
       osMessageQueuePut(fila_mensagem_cen, &aux_receiv, 0U, 0U);   
      
      }
      else if(aux_receiv[0]=='d')
      {
         if(osMessageQueueGetSpace(fila_mensagem_dir))
       osMessageQueuePut(fila_mensagem_dir, &aux_receiv, 0U, 0U);   
      
      }
      
      for(int j=0; j<16;j++)
        aux_receiv[j]='\0';
    }
  
    
    //osDelay(10);
  }
  
}

void main(void){
  UARTInit();
/*  char aux[16];
  char elevador_desliga[5]={'e','p','\r'};
  char elevador_liga[5]={'e','s','\r'};
  char elevador='e';
  while(1){

    if(UARTRxBytesAvail())
    {
      UARTgets(aux,sizeof(aux));
      UARTFlushRx();
      if (aux[0]=='e' && aux[1]=='E')
      {   
        UARTprintf("es\r");  //Desce               

      }
      else if (aux[0]=='e' && aux[1]=='I')
      {   
        UARTprintf(elevador_desliga);  //Desce               

      }
      else if (aux[0]=='c' && aux[1]=='E')
      {   
        UARTprintf("eLd\r");  //Desce               

      }
      else if (aux[0]=='c' && aux[1]=='I')
      {   
        UARTprintf("eDd\r");  //Desce               

      }
      else if (aux[0]=='d' && aux[1]=='E')
      {   
        UARTprintf("ea\r");  //Desce               

      }
      else if (aux[0]=='d' && aux[1]=='I')
      {   
        UARTprintf("ef\r");  //Desce               

      }      
    }
  }*/
  osKernelInitialize();
  uart_id = osMutexNew(NULL);
  main_thread = osThreadNew(app_main, NULL, NULL);

  fila_mensagem_esq=osMessageQueueNew(8, sizeof(char)*16,NULL);
  fila_mensagem_cen=osMessageQueueNew(8, sizeof(char)*16,NULL);
  fila_mensagem_dir=osMessageQueueNew(8, sizeof(char)*16,NULL);



  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  
  


  while(1);
  
} // main
