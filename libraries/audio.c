/* Includes ------------------------------------------------------------------*/
#include "audio.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define GPIO_CLOCK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define SOUND_DAC  hdac
#define TRIGGER_TIMER  htim6
#define BUFFER_SIZE  512

/* Set when start to convert and cleared when DAC convert completely */
static volatile bool isDacBusy; 

/* Private typedef -----------------------------------------------------------*/
/* Private function prototypes------------------------------------------------*/
/* Function implementation ---------------------------------------------------*/
/* This callback function is called automatically when DAC convert completely */
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
  isDacBusy = false;
}

void Audio_Init(uint16_t sampleRate)
{
  GPIO_CLOCK_ENABLE();
  MX_DMA_Init();
  MX_DAC_Init();
  MX_TIM6_Init(sampleRate);
}

void Play_Sound(const uint8_t *soundData, uint16_t dataLen)
{
  uint16_t dataIndex = 0; /* data index, increase from 0 to data length*/
	
  /* Length of the packet will be sent */
  uint16_t packetLen = (dataLen >= BUFFER_SIZE) ? BUFFER_SIZE : dataLen;
  isDacBusy = false;
  
  /* Start DAC and Timer */
  HAL_TIM_Base_Start(&TRIGGER_TIMER);  
  HAL_DAC_Start(&SOUND_DAC,DAC_CHANNEL_1);
  
  while(true)
  {
    if(isDacBusy == false)
    {
      /* Start DMA to transmit data */
      HAL_DAC_Start_DMA(&SOUND_DAC, DAC_CHANNEL_1, (uint32_t*)(&soundData[dataIndex]), packetLen, DAC_ALIGN_8B_R);
      isDacBusy = true;
      dataIndex += packetLen;
      
      /* If dataIndex > dataLen, then stop and exit function*/
      if(dataIndex >= dataLen)
      {
        HAL_TIM_Base_Stop(&TRIGGER_TIMER);  
        HAL_DAC_Stop(&SOUND_DAC,DAC_CHANNEL_1);
        return;
      }
      
      /* Calculate remain length and determine the next packet size*/
      uint16_t remainLen = dataLen - dataIndex;
      packetLen = (remainLen >= BUFFER_SIZE) ? BUFFER_SIZE : remainLen;
    }
  }
}

/*****************************END OF FILE***************************************/
