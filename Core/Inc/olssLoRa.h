#ifndef OLSSLORA
#define OLSSLORA

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

// -------------------------------------------------------------------------------

void StartLORA_PB14(void);

uint8_t ComandoLORA(UART_HandleTypeDef *huart, uint8_t *comando);

uint8_t ConfigLORA(UART_HandleTypeDef *huart);


// -------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
