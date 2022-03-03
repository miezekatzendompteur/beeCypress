/*******************************************************************************
* File: main.c
*
* Version: 1.0
*
* Description:
*  This example project demonstrates the basic operation of the System Tick
*  Timer: periodic LED triggering - every second and minute.
*
********************************************************************************
* Copyright 2014-2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/
#include <project.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_PRIORITY                    (3u)
#define LIGHT_OFF                       (0u)
#define LIGHT_ON                        (1u)
#define EOM_CR        0x0D    //message separator char (\r)
#define EOM_LF        0x0A    //message separator char (\n)


/* Global Variables */
uint32 msCount;
volatile uint32 intSW0 = 0;
volatile uint32 intSW1 = 0;
volatile uint32 intSW2 = 0;
volatile uint32 intSW3 = 0;

volatile bool boSW0 = false;
volatile bool boSW1 = false;
volatile bool boSW2 = false;
volatile bool boSW3 = false;

/* Function Prototypes */
void SysTickISRCallback(void);

CY_ISR(GPIO0IsrHandler)
{
    /* Clear pending Interrupt */
    isr_GPIO_0_ClearPending();
    
    /* Clear pin Interrupt */
    SW0_ClearInterrupt();

    intSW0 = msCount;
    boSW0 = true;
}

CY_ISR(GPIO1IsrHandler)
{
    /* Clear pending Interrupt */
    isr_GPIO_1_ClearPending();
    
    /* Clear pin Interrupt */
    SW1_ClearInterrupt();

    intSW1 = msCount;
    boSW1 = true;
}

CY_ISR(GPIO2IsrHandler)
{
    /* Clear pending Interrupt */
    isr_GPIO_2_ClearPending();
    
    /* Clear pin Interrupt */
    SW2_ClearInterrupt();

    intSW2 = msCount;
    boSW2 = true;
}

CY_ISR(GPIO3IsrHandler)
{
    /* Clear pending Interrupt */
    isr_GPIO_3_ClearPending();
    
    /* Clear pin Interrupt */
    SW3_ClearInterrupt();

    intSW3 = msCount;
    boSW3 = true;
}


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Starts the SysTick timer and sets the callback function that will be called
*  on SysTick interrupt. Updates timing variables at one millisecond rate and
*  prints the system time to the UART once every second in the main loop.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{
    char time[16u];
    char buffer[128u];
    uint32 byte, i;
    bool boCRLF;
    bool boStart;

    
    const int RxBufferSize = 128;
    
    char RxBuffer[RxBufferSize];     // Rx circular buffer to hold all incoming command
    char *RxWriteIndex = RxBuffer;    // pointer to position in RxBuffer to read and process bytes

    /* Initialize global variables. */
    intSW0 = 0;
    intSW1 = 0;
    intSW2 = 0;
    intSW3 = 0;
    
    boSW0 = false;
    
    boCRLF = false;
    boStart = false;
    
    msCount = 0u;
    
    /* Sets up the GPIO interrupt and enables it */
    isr_GPIO_0_StartEx(GPIO0IsrHandler);

    /* Changes initial priority for the GPIO interrupt */
    isr_GPIO_0_SetPriority(DEFAULT_PRIORITY);
    
    /* Sets up the GPIO interrupt and enables it */
    isr_GPIO_1_StartEx(GPIO1IsrHandler);

    /* Changes initial priority for the GPIO interrupt */
    isr_GPIO_1_SetPriority(DEFAULT_PRIORITY);
    
    /* Sets up the GPIO interrupt and enables it */
    isr_GPIO_2_StartEx(GPIO2IsrHandler);

    /* Changes initial priority for the GPIO interrupt */
    isr_GPIO_2_SetPriority(DEFAULT_PRIORITY);
    
    /* Sets up the GPIO interrupt and enables it */
    isr_GPIO_3_StartEx(GPIO3IsrHandler);

    /* Changes initial priority for the GPIO interrupt */
    isr_GPIO_3_SetPriority(DEFAULT_PRIORITY);

    /* Enable global interrupts. */
    CyGlobalIntEnable;

    /* Configure the SysTick timer to generate interrupt every 1 ms
    * and start its operation. Call the function before assigning the
    * callbacks as it calls CySysTickInit() during first run that 
    * re-initializes the callback addresses to the NULL pointers.
    */
    //CySysTickStart();
    CySysTickInit();
    
        /* Find unused callback slot and assign the callback. */
    for (i = 0u; i < CY_SYS_SYST_NUM_OF_CALLBACKS; ++i)
    {
        if (CySysTickGetCallback(i) == NULL)
        {
            /* Set callback */
            CySysTickSetCallback(i, SysTickISRCallback);
            break;
        }
    }

    //set LED to high
    LED_GPIO0_Write(0x1);
    LED_GPIO1_Write(0x1);
    LED_GPIO2_Write(0x1);
    LED_GPIO3_Write(0x1);
    

    PWM_1_WritePeriod(63);
    PWM_1_WriteCompare(32);
    
    CyDelay(1000);
    
    UART_Start();
    
    UART_UartPutString("BeeMill 1V1\r\n");

    for(;;)
    {

        while((byte = UART_UartGetChar()) !=0 )
        {
            if ((byte == EOM_CR) || (byte == EOM_LF))
            {
                boCRLF = true;
                byte = '\0';
            }
            else
            {
                
            }
            *RxWriteIndex = byte;
            RxWriteIndex = RxWriteIndex + 1;
            *RxWriteIndex = '\0';
		    if (RxWriteIndex >= RxBuffer + RxBufferSize) RxWriteIndex = RxBuffer;    
	    } 
        
        
        if (boCRLF == true)
        {
            boCRLF = false;
            RxWriteIndex = RxBuffer;
            sprintf(buffer, "%s", RxBuffer);
            if (strcmp ("start",buffer) == 0){
                boStart = true;
                msCount = 0;
                CySysTickEnable();
                isr_GPIO_0_Enable();
                isr_GPIO_1_Enable();
                isr_GPIO_2_Enable();
                isr_GPIO_3_Enable();
                PWM_1_Start();
            }
            if (strcmp ("stop",buffer) == 0){
                boStart = false;
                CySysTickStop();
                isr_GPIO_0_Disable();
                isr_GPIO_1_Disable();
                isr_GPIO_2_Disable();
                isr_GPIO_3_Disable();
                PWM_1_Stop();
            }
            
            if (strcmp ("reset", buffer) == 0){
                boStart = false;
                CySoftwareReset();
            }

            UART_UartPutString(buffer); 
        }
        
        /* Tick status fires only once every millisecond. */
        if(boSW0 != 0u && boStart == true)
        {
            UART_UartPutString("Input0;");
            sprintf(time, "%lu;%u\n", intSW0, LED_GPIO0_Read());
            UART_UartPutString(time);
            LED_GPIO0_Write(~LED_GPIO0_Read());
            boSW0 = false;            
        }
        
        /* Tick status fires only once every millisecond. */
        if(boSW1 != 0u && boStart == true)
        {
            UART_UartPutString("Input1;");
            sprintf(time, "%lu;%u\n", intSW1, LED_GPIO1_Read());
            UART_UartPutString(time);
            LED_GPIO1_Write(~LED_GPIO1_Read());
            boSW1 = false;            
        }
        
        /* Tick status fires only once every millisecond. */
        if(boSW2 != 0u && boStart == true)
        {
            UART_UartPutString("Input2;");
            sprintf(time, "%lu;%u\n", intSW2, LED_GPIO2_Read());
            UART_UartPutString(time);
            LED_GPIO2_Write(~LED_GPIO2_Read());
            boSW2 = false;            
        }
        
        /* Tick status fires only once every millisecond. */
        if(boSW3 != 0u && boStart == true)
        {
            UART_UartPutString("Input3;");
            sprintf(time, "%lu;%u\n", intSW3, LED_GPIO3_Read());
            UART_UartPutString(time);
            LED_GPIO3_Write(~LED_GPIO3_Read());
            boSW3 = false;            
        }
    }
}

void SysTickISRCallback(void)
{
    msCount++;
}

/* [] END OF FILE */