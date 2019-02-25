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
typedef enum
{
  INIT_STATE,
  BUF1_IDLE,
  BUF2_IDLE
}transmitState_t;

typedef struct
{
  uint16_t packetLen;
  bool isLastPacket;
}packetInfo_t;

/* Private function prototypes------------------------------------------------*/
/* Fill to buffer a new packet from soundData, depends on remainLen */
static void FillBuffer(packetInfo_t *packetInfo, uint8_t *buffer, const uint8_t *soundData, uint16_t remainLen);

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
  uint8_t buffer1[BUFFER_SIZE];
  uint8_t buffer2[BUFFER_SIZE];
  transmitState_t transmitState = BUF2_IDLE;
  uint16_t dataIndex = 0; /* data index, increase from 0 to data length*/
  packetInfo_t packetInfo; /* Info of the next packet will be sent */
  isDacBusy = false;
  
  /* Fill buffer1 */
  FillBuffer(&packetInfo, buffer1, soundData, dataLen);
  dataIndex = packetInfo.packetLen;
  HAL_TIM_Base_Start(&TRIGGER_TIMER);  
  HAL_DAC_Start(&SOUND_DAC,DAC_CHANNEL_1);
  while(true)
  {
    if(isDacBusy == false)
    {
      if(transmitState == BUF2_IDLE)
      {
        /* Transmit buffer1 and fill next data to buffer2 */
        HAL_DAC_Start_DMA(&SOUND_DAC, DAC_CHANNEL_1, (uint32_t*)buffer1, packetInfo.packetLen, DAC_ALIGN_8B_R);
        if(!packetInfo.isLastPacket)
        {
          /* This is not the last packet, fill the buffer2 */
          FillBuffer(&packetInfo, buffer2, &soundData[dataIndex], dataLen - dataIndex);
          dataIndex += packetInfo.packetLen;
          transmitState = BUF1_IDLE;
        }
        else
        {
          /* Last packet have been sent, stop DAC and exit function */
          HAL_TIM_Base_Stop(&TRIGGER_TIMER);  
          HAL_DAC_Stop(&SOUND_DAC,DAC_CHANNEL_1);
          return;
        }
      }
      else if(transmitState == BUF1_IDLE)
      {
        /* Transmit buffer2 and fill next data to buffer1 */
        HAL_DAC_Start_DMA(&SOUND_DAC, DAC_CHANNEL_1, (uint32_t*)buffer2, packetInfo.packetLen, DAC_ALIGN_8B_R);
        
        if(!packetInfo.isLastPacket)
        {
          /* This is not the last packet, fill the buffer1 */
          FillBuffer(&packetInfo, buffer1, &soundData[dataIndex], dataLen - dataIndex);
          dataIndex += packetInfo.packetLen;
          transmitState = BUF2_IDLE;
        }
        else
        {
          /* Last packet have been sent, stop DAC and exit function */
          HAL_TIM_Base_Stop(&TRIGGER_TIMER);  
          HAL_DAC_Stop(&SOUND_DAC,DAC_CHANNEL_1);
          return;
        }
      }
      else
      {
      }
    	
      isDacBusy = true;
    }
  }
}

static void FillBuffer(packetInfo_t *packetInfo, uint8_t *buffer, const uint8_t *soundData, uint16_t remainLen)
{
  /*Fill to buffer a new packet from soundData, depends on remainLen */
  if(remainLen > BUFFER_SIZE)
  {
    packetInfo->isLastPacket = false;
    packetInfo->packetLen = BUFFER_SIZE;
  }
  else
  {
    packetInfo->isLastPacket = true;
    packetInfo->packetLen = remainLen;
  }
  
  /* Fill data to buffer */
  for(int i = 0; i < packetInfo->packetLen; i++)
  {
    buffer[i] = soundData[i];
  }
}

/*****************************END OF FILE***************************************/
