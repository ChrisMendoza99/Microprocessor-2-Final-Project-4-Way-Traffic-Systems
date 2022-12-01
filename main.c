/**
 * Final Project made by Christopher A. Mendoza
 * Traffic Light System
 * UTEP 2022
 * last edited and finalized: Nov 22, 2022
 */

/*---libraries---*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"
#include <driver/dac.h>
/*---Global and Handle Initializtions*/
#define TIME_0 75                   //Total Period of the Traffic Light System
#define HIGH 1                      //Logic High 
#define LOW 0                       //Logic Low 
#define INPUT_CAP 15                //To insure that any pedestrian presses multiple times, it resets 
/*---Task Handles---*/
TaskHandle_t Task1;
/*---Semaphore Handles---*/
SemaphoreHandle_t semaphore_pedestrian = NULL; 
// SemaphoreHandle_t mySemaphore2 = NULL;
/*---Timers---*/
static TimerHandle_t NS_Traffic_Light = NULL;
static TimerHandle_t EW_Traffic_Light = NULL;
int32_t arrIndex_1 = 0;
int32_t arrIndex_2 = 0;
/*---Queue Handles---*/
QueueHandle_t pedestrian_queue;
QueueHandle_t pedestrian_assist_queue;
/*---Interrupt Flags and Buttons---*/
int32_t flag_1, flag_2, flag_3, flag_4 = 0;
#define ESP_INTR_FLAG_DEFAULT 0 //Interrupt config
/*---First four buttons are for North and South---*/
#define BUTTON_0 19
#define BUTTON_1 21
#define BUTTON_2 22
#define BUTTON_3 23
/*---First four buttons are for West and East---*/
#define BUTTON_4 35
#define BUTTON_5 34
#define BUTTON_6 39
#define BUTTON_7 36
/*---Pedestrian GPIO Interrupt Handler---*/
static void IRAM_ATTR gpio_isr_handler(void* pinData)
{
    //This gets which pin was toggled
    gpio_num_t buttonPin = (gpio_num_t) pinData;
    xQueueSendToFront(pedestrian_queue, &buttonPin,0);
}
/* Initializing the Pins needed for this program */
void GPIOInitOutput(uint8_t *Outpins, int OutSize)
{
    /*--- The for loop initializes the Outputs*/
    for(int i = 0; i < OutSize; i++)
    {
        gpio_pad_select_gpio(Outpins[i]);
        gpio_set_direction(Outpins[i], GPIO_MODE_OUTPUT);
    }
}
/*----Traffic lights for North and South----*/
void Traffic_Light_NS(TimerHandle_t xTimer)
{

    const int32_t street_time_1 = TIME_0;
    arrIndex_1 += 1;
    GPIO.out |= BIT4; //Green Timing on
    if(arrIndex_1 < 22 && flag_1 >= 1)
    {
        GPIO.out |= BIT18;
    }
    if(arrIndex_1 < 22 && flag_2 >= 1)
    {
        GPIO.out |= BIT5;
    }   
    if(arrIndex_1 > 22)
    {

        GPIO.out &= ~BIT4; //Green Timing off
        GPIO.out ^= BIT2; //Yellow Timing on
        if(flag_1 >= 1) //Flag 1 enabled
        {
            if((flag_1 >= 6 || flag_2 >= 6) && (arrIndex_1 > 22 && arrIndex_1 < 32))
            {
                xSemaphoreGive(semaphore_pedestrian);
            }
            GPIO.out |= BIT18;
        }
        if(flag_2 >= 1) //Flag 2 enabled
        {
            if((flag_1 >= 6 || flag_2 >= 6) && (arrIndex_1 > 22 && arrIndex_1 < 32))
            {
                xSemaphoreGive(semaphore_pedestrian);
            }
            GPIO.out |= BIT5;
        }
    }
    if(arrIndex_1 == 32)
    {
        flag_1 = 0; flag_2 = 0;
    }
    if(arrIndex_1 > 32)
    {
        GPIO.out &= ~BIT5;
        GPIO.out &= ~BIT18;
        GPIO.out &= ~BIT2; //Yellow Timing off
        GPIO.out |= BIT15; //Red Timing on
    }        
    if(arrIndex_1 == street_time_1)
    {
        arrIndex_1 = 0;
        GPIO.out &= ~BIT15; //Red Timing off
        GPIO.out |= BIT4; //Green Timing on   
    }
}
/*----Traffic lights for West and East----*/
void Traffic_Light_EW(TimerHandle_t xTimer)
{
    const int32_t street_time_2 = TIME_0;
    arrIndex_2 += 1;
    GPIO.out |= BIT13; //Red Timing on
    if(arrIndex_2 > 32)
    {
        GPIO.out &= ~BIT13; //Red Timing off
        GPIO.out |= BIT14; //Green Timing on
        if(flag_3 >= 1)
        {
            GPIO.out |= BIT27;
        }
        if(flag_4 >= 1)
        {
            GPIO.out |= BIT26;
        }
    }
    if(arrIndex_2 > 61)
    {
        GPIO.out &= ~BIT14; //Green Timing off
        GPIO.out ^= BIT12; //Yellow Timing on
        if(flag_3 >= 1)
        {
            if((flag_3 >= 6 || flag_4 >= 6) && (arrIndex_2 > 61 && arrIndex_2 < TIME_0))
            {
                xSemaphoreGive(semaphore_pedestrian);
            }
            GPIO.out |= BIT27;
        }
        if(flag_4 >= 1)
        {
            if((flag_3 >= 6 || flag_4 >= 6) && (arrIndex_2 > 61 && arrIndex_2 < TIME_0))
            {
                xSemaphoreGive(semaphore_pedestrian);
            }
           GPIO.out |= BIT26;
        }
    }
    if(arrIndex_2 == street_time_2)
    {
        arrIndex_2 = 0;
        flag_3 = 0; flag_4 = 0;
        GPIO.out &= ~BIT12; //Yellow Timing off
        GPIO.out &= ~BIT27;
        GPIO.out &= ~BIT26;
        GPIO.out |= BIT13; //Red Timing on
    }
}
/*---Interrupt Subroutine Setup---*/
void pedestrian_input_setup()
{
     /* ---IO configuration */
    gpio_config_t io_conf;
    
    io_conf.intr_type = GPIO_INTR_NEGEDGE;  /* Set up as Nrgative Edge */ 
    io_conf.mode = GPIO_MODE_INPUT;     /* Set pins as input */
    io_conf.pin_bit_mask = (1ULL << BUTTON_0) | (1ULL << BUTTON_1)| (1ULL << BUTTON_2)| (1ULL << BUTTON_3)| (1ULL << BUTTON_4)| (1ULL << BUTTON_5)| (1ULL << BUTTON_6)| (1ULL << BUTTON_7);  /* Add input bit mask */
    io_conf.pull_down_en = 1;   /* Enable pulldown */
    io_conf.pull_up_en = 0;     /* Disable pullup */
    /* Set configuration */
    gpio_config(&io_conf);
    /* Set default interrupt flag */
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    /*---All the buttons are set to be interruptable---*/
    gpio_isr_handler_add(BUTTON_0, gpio_isr_handler, (void*) BUTTON_0); 
    gpio_isr_handler_add(BUTTON_1, gpio_isr_handler, (void*) BUTTON_1);
    gpio_isr_handler_add(BUTTON_2, gpio_isr_handler, (void*) BUTTON_2);
    gpio_isr_handler_add(BUTTON_3, gpio_isr_handler, (void*) BUTTON_3); 
    gpio_isr_handler_add(BUTTON_4, gpio_isr_handler, (void*) BUTTON_4);
    gpio_isr_handler_add(BUTTON_5, gpio_isr_handler, (void*) BUTTON_5);
    gpio_isr_handler_add(BUTTON_6, gpio_isr_handler, (void*) BUTTON_6);
    gpio_isr_handler_add(BUTTON_7, gpio_isr_handler, (void*) BUTTON_7);  
}
/*---Looks out for a pedestrian pushing a button---*/
void Task_street_pedestrian()
{
    uint8_t rxData; //The data buffer for the recieving queue
    while(1)
    {
        if(xQueueReceive(pedestrian_queue, &rxData, (TickType_t)100) == pdPASS)
        {
            printf("GPIO %i was recieved!! [%i]\n", rxData, xTaskGetTickCount()); //and here we print the data
            /*---First four "ifs" are for North and South*/
            if(rxData == 19 || rxData == 21) //Button 2
            {
                flag_1 += 1;
                if(flag_1 == INPUT_CAP)
                    flag_1 = 6;
                printf("%i\n", flag_1);
            }
            if(rxData == 22 || rxData == 23) //Button 4
            {
                flag_2 += 1;
                if(flag_2 == INPUT_CAP)
                    flag_2 = 6;
                printf("%i\n", flag_2);
            }
            /*The rest are for East and West*/
            if(rxData == 35 || rxData == 34) //Button 1
            {
                flag_3 += 1;
                if(flag_3 == INPUT_CAP)
                    flag_3 = 6;
                printf("%i\n", flag_3);
            }
            if(rxData == 39 || rxData == 36) //Button 5
            {
                flag_4+= 1;
                if(flag_4 == INPUT_CAP)
                    flag_4 = 6;
                printf("%i\n", flag_4);
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
/*---The task that will handle toggling the buzzer for however long it takes---*/
void task_pedestrian_assistance()
{   
    /* Enable DAC output through channel 1 */
    dac_output_enable(DAC_CHANNEL_1);
    while(1)
    {
        /* wait for 1000 ticks to receive semaphore */
         if(xSemaphoreTake(semaphore_pedestrian, 75/portTICK_RATE_MS) == pdTRUE)
         {
            dac_output_voltage(DAC_CHANNEL_1, 255); 
            vTaskDelay(100/portTICK_PERIOD_MS); 
            dac_output_voltage(DAC_CHANNEL_1, 0);   
            vTaskDelay(100/portTICK_PERIOD_MS);
         }
         else
         {
            vTaskDelay(100/portTICK_RATE_MS); /* 100 ms delay */
         }
    }
}
void app_main(void)
{
    /*---Pins for both the leds, and buttons---*/
    uint8_t OutPins[] = {15,2,4,13,12,14,18,5,27,26};
    int OutSize = sizeof(OutPins) / sizeof(uint8_t);
    GPIOInitOutput(OutPins, OutSize);
    /*---Setting up the Interrupt Services---*/
    pedestrian_input_setup();
    /*---Queue Creation(pending use)---*/
    pedestrian_queue = xQueueCreate(5, sizeof(int));
    /*---Semaphore Binary---*/
    semaphore_pedestrian = xSemaphoreCreateBinary();  
    /*---Creating the Timers for the Traffic Lights for North and South---*/
    NS_Traffic_Light = xTimerCreate("Traffic_Light_NW",1000/portTICK_PERIOD_MS,pdTRUE,(void*)0,Traffic_Light_NS);
    EW_Traffic_Light = xTimerCreate("Traffic_Light_EW",1000/portTICK_PERIOD_MS,pdTRUE,(void*)1,Traffic_Light_EW);
    /*--Creating Task*/
    xTaskCreate(Task_street_pedestrian,"Task_1",2048,NULL,5, Task1);
    xTaskCreate(task_pedestrian_assistance,"Task 2", 2048,NULL, 5, NULL);    
    /*---Timer starts!---*/
    xTimerStart(NS_Traffic_Light, portMAX_DELAY);
    xTimerStart(EW_Traffic_Light, portMAX_DELAY);
} 