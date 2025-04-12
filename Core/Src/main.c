/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "olssLoRa.h"
                                                                      //#include "liquidcrystal_i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

int16_t RSSI = 0;
int16_t SNR = 0;
int16_t distancia = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void EncenderLEDuC(void){ HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_RESET); }

void ApagarLEDuC(void){	HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_SET); }

void ToggleLEDuC(void){ HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin); }

uint8_t setTipoNodo(void){ // Para decidir si el dispositivo es la antena (devuelve 0) o el emisor (devuelve 1)
	// Si el botón "KEY" se pulsa rápidamente, será antena. Si se pulsa durante >2s, será emisor
	EncenderLEDuC();
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != 0){} // Esperamos a que se pulse el botón "KEY"
	ApagarLEDuC();
	HAL_Delay(2000);
	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != 0){ // ANTENA
		EncenderLEDuC(); // 1 blink del led azul indica que se ha seleccionado antena
		HAL_Delay(500);
		ApagarLEDuC();
		HAL_Delay(500);
		return 0;
	}
	else{ // EMISOR
		EncenderLEDuC(); // 2 blinks del led azul indican que se ha seleccionado emisor
		HAL_Delay(500);
		ApagarLEDuC();
		HAL_Delay(500);
		EncenderLEDuC();
		HAL_Delay(500);
		ApagarLEDuC();
		HAL_Delay(500);
		return 1;
	}
}

void detectRYLR998error(uint8_t codError){ // Mecanismo para indicar que hay un error en la comunicación con el módulo RYLR998
	if((codError != 0) && (codError != 100)){
		while(1){ // Si hay un error de comunicación, entramos en un bucle infinito en el que el led azul no para de parpadear
			ToggleLEDuC();
			HAL_Delay(250);
		}
	}
}

void aplicacionEmisor1(void){

	uint8_t mensajeInicial[] = {"HOLA"}; // Primer mensaje que manda el emisor. Sin datos del RSSI o SNR en él
	uint8_t recibido[TAM_RX_BUFFER] = {0}; // Buffer de recepción de datos

	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != 0){} // Esperamos hasta que se pulse el botón "KEY"
	EncenderLEDuC(); // Indicamos que se ha detectado la pulsación
	HAL_Delay(250);
	ApagarLEDuC();
	if(SendLoRaMS(&huart1, mensajeInicial, sizeof(mensajeInicial)-1, 6) == 0){
	  EncenderLEDuC(); // Indicamos que estamos esperando datos
	  while(RecLoRaTIMEOUT(&huart1, recibido) == 30){
		  ApagarLEDuC(); // Indicamos que estamos intentando sincronizarnos con la antena
		  SendLoRaMS(&huart1, mensajeInicial, sizeof(mensajeInicial)-1, 6);
		  EncenderLEDuC();
	  }
	  ApagarLEDuC(); // Indicamos que se han recibido los datos de vuelta
	  RSSI = getRSSI(recibido, 1);
	  SNR = getSNR(recibido, 1);
	  distancia = getDistancia(RSSI, SNR);
	}
    HAL_Delay(1000); // Para evitar errores de falta de sincronización
}

void aplicacionAntena1(void){

	uint8_t recibido[TAM_RX_BUFFER] = {0}; // Buffer de recepción de datos
	uint8_t mensajeConDatos[9] = {0}; // Vector para almacenar el mensaje creado con los datos del RSSI y SNR
	uint8_t tamMsgCD = 0; // Tamaño del mensaje que contiene datos de RSSI y SNR

	EncenderLEDuC(); // Indicamos que estamos esperando datos
	while(RecLoRa(&huart1, recibido) != 0){}
    ApagarLEDuC(); // Indicamos que los datos han sido recibidos
	RSSI = getRSSI(recibido, 0);
	SNR = getSNR(recibido, 0);
	tamMsgCD = construirMsg_RSSI_SNR(mensajeConDatos, RSSI, SNR);
	HAL_Delay(3000);
	SendLoRaMS(&huart1, mensajeConDatos, tamMsgCD, 5);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  uint8_t esEmisor = 0; // 1 si es antena, 0 si es emisor
  uint8_t errorRYLR998 = 100; // 100 es el estado de inicialización de la variable de gestión de errores de comunicación UART con el módulo RYLR998

  esEmisor = setTipoNodo(); // Se determina si el dispositivo es el emisor o la antena

  if(esEmisor == 1){ errorRYLR998 = ConfigLoRaModule(&huart1, 5, 9);} // Configuración del emisor
  else{ errorRYLR998 = ConfigLoRaModule(&huart1, 6, 9); } // Configuración de la antena

  detectRYLR998error(errorRYLR998); // Comprobamos que no haya errores en la comunicación UART con el RYLR998

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(esEmisor == 1){ // Código para el emisor
		  aplicacionEmisor1();
	  }
	  else{ // Código para la antena
		  aplicacionAntena1();
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  } // ------------------- FIN WHILE(1) ------------------------------
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : BLUE_LED_Pin */
  GPIO_InitStruct.Pin = BLUE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BLUE_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
