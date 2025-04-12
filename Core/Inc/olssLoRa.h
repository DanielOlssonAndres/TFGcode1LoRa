#ifndef OLSSLORA
#define OLSSLORA

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "math.h"
#include <stdio.h>
#include <string.h>

// -------------------------------------------------------------------------------

#define TAM_MENSAJE_ESPERADO_RX 9 // Para optimizar la memoria
#define NUM_MUESTRAS 6 // Número de medidas del RSSI y SNR que se toman
#define TAM_RX_BUFFER NUM_MUESTRAS * (TAM_MENSAJE_ESPERADO_RX + 20) // Tamaño para el buffer de recepción

#define MENSAJE_CON_DATOS 1 // 1: Activar detección de datos en mensaje recibido. 0: Funcionalidad desactivada


uint8_t ComandoLoRaModule(UART_HandleTypeDef *huart, uint8_t *comando, uint8_t tam);

uint8_t TestLoRaModule(UART_HandleTypeDef *huart);

uint8_t ConfigLoRaModule(UART_HandleTypeDef *huart, uint8_t addr, uint8_t net);

uint8_t SendLoRa(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam, uint8_t addr);

uint8_t SendLoRaMS(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam, uint8_t addr);

uint8_t RecLoRa(UART_HandleTypeDef *huart, uint8_t* bufferRx);

uint8_t RecLoRaTIMEOUT(UART_HandleTypeDef *huart, uint8_t* bufferRx);

int16_t extraerMsg_RSSI(uint8_t* mensaje);

int16_t extraerMsg_SNR(uint8_t* mensaje);

int16_t filtradoDatos(int16_t* datos, uint8_t tam);

int16_t getRSSI(uint8_t* bufferRx, uint8_t contarDatosEnMsg);

int16_t getSNR(uint8_t* bufferRx, uint8_t contarDatosEnMsg);

uint8_t construirMsg_RSSI_SNR(uint8_t* mensaje, int16_t RSSI, int16_t SNR);

int16_t getDistancia(int16_t RSSI, int16_t SNR);

// -------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
