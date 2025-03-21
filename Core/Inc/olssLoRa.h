#ifndef OLSSLORA
#define OLSSLORA

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

// -------------------------------------------------------------------------------

#define MAX_TAM_MENSAJE 23 // El máximo tamaño permitido del mensaje a enviar es de 9

uint8_t ComandoLoRaModule(UART_HandleTypeDef *huart, uint8_t *comando, uint8_t tam);

uint8_t TestLoRaModule(UART_HandleTypeDef *huart);

uint8_t ConfigLoRaModule(UART_HandleTypeDef *huart, uint8_t addr, uint8_t net);

uint8_t SendLoRa(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam);

uint8_t RecLoRa(UART_HandleTypeDef *huart, uint8_t* bufferRx, uint8_t* mensajeEsperado);

// -------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
