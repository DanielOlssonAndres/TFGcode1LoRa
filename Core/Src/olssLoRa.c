#include "olssLoRa.h"

uint8_t ComandoLoRaModule(UART_HandleTypeDef *huart, uint8_t* comando, uint8_t tam){

	  uint8_t RXbuffer[6] = {0}; // Buffer para la recepción de +OK\n\r
	  uint8_t indicador = 0;

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	  HAL_Delay(200);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	  HAL_Delay(200);

	  HAL_UART_Transmit(huart, (uint8_t *)comando, tam , HAL_MAX_DELAY);
	  HAL_UART_Receive(huart, RXbuffer, sizeof(RXbuffer), HAL_MAX_DELAY);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

	  if((RXbuffer[1] == '+') && (RXbuffer[2] == 'O') && (RXbuffer[3] == 'K')){
		  indicador = 1;
	  }

	  return indicador; // 1 si todo OK, 0 si error
}

uint8_t TestLoRaModule(UART_HandleTypeDef *huart){

	uint8_t CMDtest[] = {"AT\r\n"};

	return ComandoLoRaModule(huart, CMDtest, sizeof(CMDtest)-1);
}

uint8_t ConfigLoRaModule(UART_HandleTypeDef *huart, uint8_t addr, uint8_t net){

	uint8_t todoOK = 0;

	uint8_t CMDaddress[14] = {"AT+ADDRESS="};
	CMDaddress[11] = addr + '0';
	CMDaddress[12] = '\r';
	CMDaddress[13] = '\n';
	uint8_t CMDnetwork[16] = {"AT+NETWORKID="};
	CMDnetwork[13] = net + '0';
	CMDnetwork[14] = '\r';
	CMDnetwork[15] = '\n';
	uint8_t CMDband[] = {"AT+BAND=868500000,M\r\n"};
	uint8_t CMDparam[] = {"AT+PARAMETER=9,7,1,12\r\n"};
	uint8_t CMDrfpower[] = {"AT+CRFOP=14\r\n"};

	todoOK = todoOK + ComandoLoRaModule(huart, CMDaddress, sizeof(CMDaddress));
	todoOK = todoOK + ComandoLoRaModule(huart, CMDnetwork, sizeof(CMDnetwork));
	todoOK = todoOK + ComandoLoRaModule(huart, CMDband, sizeof(CMDband)-1);
	todoOK = todoOK + ComandoLoRaModule(huart, CMDparam, sizeof(CMDparam)-1);
	todoOK = todoOK + ComandoLoRaModule(huart, CMDrfpower, sizeof(CMDrfpower)-1);

	if(todoOK == 5){todoOK = 1;}
	else{todoOK = 0;}

	return todoOK;
}

uint8_t SendLoRa(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam){

	uint8_t mensajeOK[MAX_TAM_MENSAJE] = {"AT+SEND=5,"};
	uint8_t i;

	mensajeOK[10] = tam + '0'; // Convertimos a formato ASCII
	mensajeOK[11] = ',';
	for(i = 0; i < tam; i++){mensajeOK[i + 12] = mensaje[i];}
	mensajeOK[12 + tam] = '\r';
	mensajeOK[13 + tam] = '\n';

	return ComandoLoRaModule(huart, mensajeOK, 14 + tam);
}

uint8_t RecLoRa(UART_HandleTypeDef *huart, uint8_t* bufferRx, uint8_t* mensajeEsperado){

}


