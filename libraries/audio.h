/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_H
#define __AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Init before play sound
  * @param  sampleRate: sample rate of the file
  * @retval None
  */
void Audio_Init(uint16_t sampleRate);

/**
  * @brief  Play a certain sound.
  * @param  soundData: sound data in PCM format, mono, 8bits per sample.
  * @retval Error code
  */
void Play_Sound(const uint8_t *soundData, uint16_t dataLen);

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_H */

/******************************END OF FILE*************************************/
