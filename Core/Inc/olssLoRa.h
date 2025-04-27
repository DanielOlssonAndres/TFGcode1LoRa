#ifndef OLSSLORA
#define OLSSLORA

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stm32f4xx_hal.h"
#include "math.h"
#include <stdio.h>
#include <string.h>
#include "i2c_lcd.h" // Librería externa para usar el LCD I2C Display

// -------------------------------------------------------------------------------

#define TAM_MENSAJE_ESPERADO_RX 9 // Para optimizar la memoria
#define NUM_MUESTRAS 6 // Número de medidas del RSSI y SNR que se toman
#define TAM_RX_BUFFER NUM_MUESTRAS * (TAM_MENSAJE_ESPERADO_RX + 20) // Tamaño para el buffer de recepción
#define MENSAJE_CON_DATOS 1 // 1: Activar detección de datos en mensaje recibido. 0: Funcionalidad desactivada
#define SNR_MAX 100 // Limita el valor del SNR a un máximo para evitar errores (suele ser <15)
#define SNR_MIN -10 // Limita el valor del SNR a un mínimo, para no tner en cuenta medidas con un SNR peor
#define PARAM_A -23 // Valor de referencia del RSSI a un metro de distancia (por defecto)
#define PARAM_n 2.57 // Exponente de pérdida para el cálculo de la distancia a partir del RSSI (por defecto)
#define METROS_CALIBRAR_n 50 // A qué distancia se va a pedir al usuario que se situe para calibrar "n" (60 óptimo para zona de pruebas)
#define FACTOR_SEGURIDAD_DISTANCIA 2.5 // Factor de seguridad para dar márgenes en el cálculo de la distancia a partir del RSSI

void EncenderLEDuC(void);

void ToggleLEDuC(void);

void ApagarLEDuC(void);

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

int16_t getRSSI(uint8_t* bufferRx, uint8_t hayDatosEnMsg);

int16_t getSNR(uint8_t* bufferRx, uint8_t hayDatosEnMsg);

uint8_t construirMsg_RSSI_SNR(uint8_t* mensaje, int16_t RSSI, int16_t SNR);

void getDistancia(int16_t* vectorDistancia, int16_t RSSI, int16_t SNR, float A, float n);

uint8_t setTipoNodo(void);

void detectRYLR998error(uint8_t codError);

void initDisplay(I2C_LCD_HandleTypeDef* lcd1, I2C_HandleTypeDef* hi2c1);

void putStringDisplay(I2C_LCD_HandleTypeDef* lcd, char* linea1, char* linea2);

void floatToChar_n(char* texto, uint8_t tamTexto, float num);

void putDataDisplay(I2C_LCD_HandleTypeDef* lcd, int16_t* vectorDistancia, int16_t RSSI, int16_t SNR);

void calibrarParametros(UART_HandleTypeDef *huart, I2C_LCD_HandleTypeDef* lcd1, float* parametros);

uint8_t configAplicacion1(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1, float* parametros);

void aplicacionEmisor1(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1, float* parametros, int16_t* vectorDistancia);

void aplicacionAntena1(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1);

// -------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
