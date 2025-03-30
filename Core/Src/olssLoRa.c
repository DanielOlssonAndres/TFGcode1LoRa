#include "olssLoRa.h"

uint8_t ComandoLoRaModule(UART_HandleTypeDef *huart, uint8_t* comando, uint8_t tam){

	  uint8_t RXbuffer[6] = {0}; // Buffer para la recepción de +OK\r\n
	  uint8_t indicador = 0;

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	  HAL_Delay(150);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	  HAL_Delay(150);

	  HAL_UART_Transmit(huart, (uint8_t *)comando, tam , HAL_MAX_DELAY);
	  HAL_UART_Receive(huart, RXbuffer, sizeof(RXbuffer), HAL_MAX_DELAY);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

	  if(RXbuffer[3] != 'R'){
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
	uint8_t CMDband[] = {"AT+BAND=868500000,M\r\n"}; //Predefinido por ley Europea
	uint8_t CMDparam[] = {"AT+PARAMETER=9,7,1,12\r\n"};
	uint8_t CMDrfpower[] = {"AT+CRFOP=14\r\n"}; //Predefinido por ley Europea

	todoOK = todoOK + ComandoLoRaModule(huart, CMDaddress, sizeof(CMDaddress));
	todoOK = todoOK + ComandoLoRaModule(huart, CMDnetwork, sizeof(CMDnetwork));
	todoOK = todoOK + ComandoLoRaModule(huart, CMDband, sizeof(CMDband)-1);
	todoOK = todoOK + ComandoLoRaModule(huart, CMDparam, sizeof(CMDparam)-1);
	todoOK = todoOK + ComandoLoRaModule(huart, CMDrfpower, sizeof(CMDrfpower)-1);

	if(todoOK == 5){todoOK = 1;}
	else{todoOK = 0;}

	return todoOK;
}

uint8_t SendLoRa(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam, uint8_t addr){

	uint8_t mensajeOK[120] = {"AT+SEND="};
	uint8_t i;

	mensajeOK[8] = addr + '0';
	mensajeOK[9] = ',';
	if(tam < 10){
		mensajeOK[10] = tam + '0';
		mensajeOK[11] = ',';
		for(i = 0; i < tam; i++){mensajeOK[i + 12] = mensaje[i];}
		mensajeOK[12 + tam] = '\r';
		mensajeOK[13 + tam] = '\n';
	}
	if((tam >= 10) && (tam < 100)){
		mensajeOK[10] = (uint8_t)(tam/10) + '0';
		mensajeOK[11] = (tam - ((uint8_t)(tam/10) * 10)) + '0';
		mensajeOK[12] = ',';
		for(i = 0; i < tam; i++){mensajeOK[i + 13] = mensaje[i];}
		mensajeOK[14 + tam] = '\r';
		mensajeOK[15 + tam] = '\n';
	}

	return ComandoLoRaModule(huart, mensajeOK, 14 + tam);
}

uint8_t SendLoRaMS(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam, uint8_t addr){

	uint8_t indicador = 0;
	uint8_t i;

	for(i = 0; i < NUM_MUESTRAS + 1; i++){
		indicador = indicador + SendLoRa(huart, mensaje, tam, addr);
	}

	if(indicador == i){ indicador = 1; }

	return indicador;
}

uint8_t RecLoRa(UART_HandleTypeDef *huart, uint8_t* bufferRx){

	uint8_t indicador = 0;
	uint8_t rxbyte = 0;
	uint8_t i = 0;
	uint8_t SM = 0;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_Delay(150);

	for(; SM < NUM_MUESTRAS; SM++){
		do{
			HAL_UART_Receive(huart, &rxbyte, 1, HAL_MAX_DELAY);
			bufferRx[i] = rxbyte;
			i++;
		}while(rxbyte != '\n');
	}

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

	if(bufferRx[3] != 'R'){
		indicador = 1;
	}

	return indicador; // 1 si todo OK, 0 si error

}

uint8_t RecLoRaTIMEOUT(UART_HandleTypeDef *huart, uint8_t* bufferRx){

	uint8_t rxbyte = 0;
	uint8_t indicador = 0;
	uint8_t SM = 0;
	uint8_t i = 0;
	HAL_StatusTypeDef status;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_Delay(150);

	for(; SM < NUM_MUESTRAS; SM++){
		do{
			status = HAL_UART_Receive(huart, &rxbyte, 1, 2000);
			bufferRx[i] = rxbyte;
			i++;

			if(status == HAL_TIMEOUT){ SM = NUM_MUESTRAS; }

		}while(rxbyte != '\n' && status != HAL_TIMEOUT);
	}


	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

	if((status != HAL_TIMEOUT) && (bufferRx[3] != 'R')){
		indicador = 1;
	}

	return indicador; // 1 si todo OK, 0 si error
}

int16_t getRSSI(uint8_t* bufferRx){

	int16_t RSSI[15] = {0};
	int16_t RSSImed = 0;
	uint8_t j = 0;
	uint8_t i = 0;
	uint8_t x = 0;

	for(; x < NUM_MUESTRAS ; x++){

		while(bufferRx[i] != '\r'){ i++; }
		while(bufferRx[i] != ','){ i--; }
		i--;
		for(; bufferRx[i] != ','; i--){
			if(bufferRx[i] != '-'){
				RSSI[x] = RSSI[x] + ((bufferRx[i] - '0') * (int16_t)pow(10, j));
				j++;
			}
			if(bufferRx[i] == '-'){
				RSSI[x] = RSSI[x] * (-1);
			}
		}

		j = 0;
		while(bufferRx[i] != '\r'){i++; }
		i++;
	}

	for(x = 0 ; x < NUM_MUESTRAS ; x++){
		RSSImed = RSSImed + RSSI[x];
	}
	RSSImed = (int16_t)(RSSImed / NUM_MUESTRAS);

	return RSSImed;
}

int16_t getSNR(uint8_t* bufferRx){

	int16_t SNR[15] = {0};
	int16_t SNRmed = 0;
	uint8_t j = 0;
	uint8_t i = 0;
	uint8_t x = 0;

	for(; x < NUM_MUESTRAS ; x++){

		while(bufferRx[i] != '\r'){ i++; }
		i--;
		for(; bufferRx[i] != ','; i--){
			if(bufferRx[i] != '-'){
				SNR[x] = SNR[x] + ((bufferRx[i] - '0') * (int16_t)pow(10, j));
				j++;
			}
			if(bufferRx[i] == '-'){
				SNR[x] = SNR[x] * (-1);
			}
		}

		j = 0;
		while(bufferRx[i] != '\r'){i++; }
		i++;
	}

	for(x = 0 ; x < NUM_MUESTRAS ; x++){
		SNRmed = SNRmed + SNR[x];
	}
	SNRmed = (int16_t)(SNRmed / NUM_MUESTRAS);

	return SNRmed;
}


