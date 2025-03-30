#ifndef OLSSLORA
#define OLSSLORA

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "math.h"

// -------------------------------------------------------------------------------

#define TAM_MENSAJE_ESPERADO_RX 9
#define NUM_MUESTRAS 2
#define TAM_RX_BUFFER NUM_MUESTRAS * (TAM_MENSAJE_ESPERADO_RX + 20) // Tamaño para el buffer de recepción

uint8_t ComandoLoRaModule(UART_HandleTypeDef *huart, uint8_t *comando, uint8_t tam);

uint8_t TestLoRaModule(UART_HandleTypeDef *huart);

uint8_t ConfigLoRaModule(UART_HandleTypeDef *huart, uint8_t addr, uint8_t net);

uint8_t SendLoRa(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam, uint8_t addr);

uint8_t SendLoRaMS(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam, uint8_t addr);

uint8_t RecLoRa(UART_HandleTypeDef *huart, uint8_t* bufferRx);

uint8_t RecLoRaTIMEOUT(UART_HandleTypeDef *huart, uint8_t* bufferRx);

int16_t getRSSI(uint8_t* bufferRx);

int16_t getSNR(uint8_t* bufferRx);


// -------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
