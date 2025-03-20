#include "olssLoRa.h"

uint8_t CMDtest[4] = {'A', 'T', '\r', '\n'};

void StartLORA_PB14(void){
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	  HAL_Delay(200);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	  HAL_Delay(200);
}

uint8_t ComandoLORA(UART_HandleTypeDef *huart, uint8_t* comando){
	  uint8_t RXbuffer[5] = {0}; // Buffer para la recepción de +OK\n\r
	  uint8_t indicador = 0;

	  HAL_UART_Transmit(huart, (uint8_t *)comando, sizeof(comando), HAL_MAX_DELAY);
	  HAL_UART_Receive(huart, RXbuffer, 5, HAL_MAX_DELAY);

	  if((RXbuffer[1] == '+') && (RXbuffer[2] == 'O') && (RXbuffer[3] == 'K')){
		  indicador = 1;
	  }

	  return indicador; // 1 si todo OK, 0 si error
}

uint8_t ConfigLORA(UART_HandleTypeDef *huart){
	uint8_t todoOK = 0;
	uint8_t cmd[4] = {'A', 'T', '\r', '\n'};
	todoOK = todoOK + ComandoLORA(huart, cmd);

	return todoOK;
}


