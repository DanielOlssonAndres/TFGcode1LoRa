#include "olssLoRa.h"

uint8_t ComandoLoRaModule(UART_HandleTypeDef *huart, uint8_t* comando, uint8_t tam){

	  uint8_t RXbuffer[16] = {0}; // Buffer para la recepción de +OK\r\n
	  uint8_t rxbyte = 0;
	  uint8_t indicador = 0;
	  uint8_t i = 0;

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
	  HAL_Delay(150); // Nos aseguramos que el módulo ha estado deshabilitado por el menos 100ms
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // Habilitamos para la comunicación
	  HAL_Delay(150);

	  HAL_UART_Transmit(huart, (uint8_t *)comando, tam , HAL_MAX_DELAY); // Transmisión del comando

	  do{ // Recibimos la respuesta byte a byte
		HAL_UART_Receive(huart, &rxbyte, 1, HAL_MAX_DELAY);
		RXbuffer[i] = rxbyte;
		i++;
	  }while(rxbyte != '\n');

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // Dehabilitamos la comunicación

	  if((RXbuffer[2] == 'E') && (RXbuffer[3] == 'R') && (RXbuffer[4] == 'R')){ // Comprobamos si la comunicación ha sido exitosa
		  // Extraemos el código de error (1 <= Código Error <= 20)
		  if(RXbuffer[7] == '\r'){ indicador = RXbuffer[6] - '0'; }
		  else{ indicador = (RXbuffer[6] - '0')*10 + (RXbuffer[7] - '0'); }
	  }

	  return indicador; // Devolvemos 0 si OK, código de error (>0) en caso de error
}

uint8_t TestLoRaModule(UART_HandleTypeDef *huart){

	uint8_t CMDtest[] = {"AT\r\n"};

	return ComandoLoRaModule(huart, CMDtest, sizeof(CMDtest)-1); // Devolvemos 0 si OK, código de error (>0) en caso de error
}

uint8_t ConfigLoRaModule(UART_HandleTypeDef *huart, uint8_t addr, uint8_t net){

	uint8_t indicador = 0;
    uint8_t buffer[32];

    // Dirección
    sprintf((char*)buffer, "AT+ADDRESS=%d\r\n", addr);
    indicador = ComandoLoRaModule(huart, buffer, strlen((char*)buffer));
    if(indicador > 0){ return indicador; }

    // Red
    sprintf((char*)buffer, "AT+NETWORKID=%d\r\n", net);
    indicador = ComandoLoRaModule(huart, buffer, strlen((char*)buffer));
    if(indicador > 0){ return indicador; }

    // Banda
    strcpy((char*)buffer, "AT+BAND=868500000,M\r\n");
    indicador = ComandoLoRaModule(huart, buffer, strlen((char*)buffer));
    if(indicador > 0){ return indicador; }

    // Parámetros de la comunicación LoRa
    strcpy((char*)buffer, "AT+PARAMETER=9,7,1,12\r\n"); //...................................................
    indicador = ComandoLoRaModule(huart, buffer, strlen((char*)buffer));
    if(indicador > 0){ return indicador; }

    // Potencia
    strcpy((char*)buffer, "AT+CRFOP=14\r\n");
    indicador = ComandoLoRaModule(huart, buffer, strlen((char*)buffer));
    if(indicador > 0){ return indicador; }

    return indicador; // Devolvemos 0 si OK, código de error (>0) en caso de error
}

uint8_t SendLoRa(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam, uint8_t addr){

	uint8_t mensajeOK[128];

    int TamInicio = sprintf((char*)mensajeOK, "AT+SEND=%d,%d,", addr, tam); // Estructura de comando AT
    memcpy(&mensajeOK[TamInicio], mensaje, tam); // Mensaje que se quiere mandar
    mensajeOK[TamInicio + tam] = '\r'; // Estructura de comando AT
    mensajeOK[TamInicio + tam + 1] = '\n'; // Estructura de comando AT

    return ComandoLoRaModule(huart, mensajeOK, TamInicio + tam + 2); // Devolvemos 0 si OK, código de error (>0) en caso de error
}

uint8_t SendLoRaMS(UART_HandleTypeDef *huart, uint8_t* mensaje, uint8_t tam, uint8_t addr){

	uint8_t indicador = 0;
	uint8_t i;

	for(i = 0; i < NUM_MUESTRAS + (uint8_t)(NUM_MUESTRAS/2); i++){
		indicador = SendLoRa(huart, mensaje, tam, addr);
	    if(indicador > 0){ return indicador; }
	}

	return indicador; // Devolvemos 0 si OK, código de error (>0) en caso de error
}

uint8_t RecLoRa(UART_HandleTypeDef *huart, uint8_t* bufferRx){

	uint8_t indicador = 0;
	uint8_t rxbyte = 0;
	uint8_t i = 0;
	uint8_t SM = 0;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // Habilitación del módulo para la comunicación
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

	if((bufferRx[2] == 'E') && (bufferRx[3] == 'R') && (bufferRx[4] == 'R')){ // Comprobamos si la comunicación ha sido exitosa
		// Extraemos el código de error (1 <= Código Error <= 20)
		if(bufferRx[7] == '\r'){ indicador = bufferRx[6] - '0'; }
		else{ indicador = (bufferRx[6] - '0')*10 + (bufferRx[7] - '0'); }
	}

	return indicador; // Devolvemos 0 si OK, código de error (>0) en caso de error
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
			status = HAL_UART_Receive(huart, &rxbyte, 1, 6000);
			bufferRx[i] = rxbyte;
			i++;

			if(status == HAL_TIMEOUT){ SM = NUM_MUESTRAS; }

		}while(rxbyte != '\n' && status != HAL_TIMEOUT);
	}

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

	if(status == HAL_TIMEOUT){ indicador = 30; } // 30 es nuestro código (no de comandos AT) para indicar TIMEOUT

	if((bufferRx[2] == 'E') && (bufferRx[3] == 'R') && (bufferRx[4] == 'R')){ // Comprobamos si la comunicación ha sido exitosa
		// Extraemos el código de error (1 <= Código Error <= 20)
		if(bufferRx[7] == '\r'){ indicador = bufferRx[6] - '0'; }
		else{ indicador = (bufferRx[6] - '0')*10 + (bufferRx[7] - '0'); }
	}

	return indicador; // Devolvemos 0 si OK, código de error (>0) en caso de error
}

int16_t extraerMsg_RSSI(uint8_t* mensaje){

	uint8_t i = 0, j = 0;
	uint8_t flag = 0;
	int16_t RSSI = 0;

	while(flag != 2){ // Nos situamos en el comienzo del RSSI
		if(mensaje[i] == ','){ flag++; }
		i++;
	}
	while(mensaje[i] != 'Y'){ i++; } // Nos situamos en el final del RSSI
	i--; // Nos situamos en la cifra de las unidades del RSSI

	for(; (mensaje[i] != ',') && (mensaje[i] != '-'); i--){
		RSSI = RSSI + ((mensaje[i] - '0') * (int16_t)pow(10, j));
		j++;
	}

	if(mensaje[i] == '-'){ RSSI = RSSI * (-1); } // Gestionamos el signo negativo

	return RSSI;
}

int16_t extraerMsg_SNR(uint8_t* mensaje){

	uint8_t i = 0, j = 0;
	uint8_t flag = 0;
	int16_t SNR = 0;

	do{
		i++;
		if(mensaje[i] == ','){ flag++; }
	}while(flag != 3);
	i--; // Nos situamos en las unidades del SNR

	for(; (mensaje[i] != 'Y') && (mensaje[i] != '-'); i--){
		SNR = SNR + ((mensaje[i] - '0') * (int16_t)pow(10, j));
		j++;
	}

	if(mensaje[i] == '-'){ SNR = SNR * (-1); } // Gestionamos el signo negativo

	return SNR;
}

int16_t filtradoDatos(int16_t* datos, uint8_t tam){

	uint8_t i, j;
	int16_t aux = 0;

	// Si solo hay una medida, no se hace nada
	if(tam == 1){ return datos[0]; }

	// Si hay 2 medidas, se hace una media aritmética
	if(tam == 2){ return (int16_t)((datos[0] + datos[1]) / 2); }

	// Si hay 3 medidas, se hace la mediana
	if(tam == 3){
	    if ((datos[0] >= datos[1] && datos[0] <= datos[2]) || (datos[0] <= datos[1] && datos[0] >= datos[2])){ return datos[0]; }
	    if ((datos[1] >= datos[0] && datos[1] <= datos[2]) || (datos[1] <= datos[0] && datos[1] >= datos[2])){ return datos[1]; }
	    return datos[2];
	}

	if(tam >= 4){

		// Si hay 4 o más medidas, lo primero es ordenar los datos
	    for (i = 0; i < tam - 1; i++) {
	        for (j = i + 1; j < tam; j++) {
	            if (datos[j] < datos[i]) {
	                aux = datos[i];
	                datos[i] = datos[j];
	                datos[j] = aux;
	            }
	        }
	    }

	    // Si tenemos 4 datos, eliminamos los extremos y hacemos una media aritmética de los datos centrales
		if(tam == 4){ return (int16_t)((datos[1] + datos[2])/2); }

	    // Si tenemos 5 datos, eliminamos los extremos y hacemos una media aritmética de los datos centrales
		if(tam == 5){ return (int16_t)((datos[1] + datos[2] + datos[3])/3); }

		// Si tenemos 6 o más datos, usamos el método de filtrado por rango intercuartil
		if(tam >= 6){

			// Calculamos Q1 y Q3
			int16_t Q1 = datos[tam / 4];
	        int16_t Q3 = datos[(3 * tam) / 4];
	        int16_t IQR = Q3 - Q1;

	        int16_t lim_inf = Q1 - 1.5 * IQR;
	        int16_t lim_sup = Q3 + 1.5 * IQR;

	        // Eliminamos los datos que se salgan de los límites y sumamos los que hayan sido aceptados
	        int32_t suma = 0;
	        uint8_t cont = 0;
	        for(i = 0; i < tam; i++){
	            if(datos[i] >= lim_inf && datos[i] <= lim_sup){
	                suma += datos[i];
	                cont++;
	            }
	        }

	        // Si tras el filtrado no quedan datos, usamos una mediana de todos los datos sin filtrar
	        if(cont == 0){
	            if(tam % 2 == 0){ // Par
	                return (int16_t)((datos[tam/2 - 1] + datos[tam/2]) / 2);
	            } else { // Impar
	                return datos[tam/2];
	            }
	        }

	        // Si tras el filtrado sí quedan datos, se hace la media aritmética
	        return (int16_t)(suma / cont);
		}
	}

	return 0; // Para evitar errores y "warnings" en el programa
}

int16_t getRSSI(uint8_t* bufferRx, uint8_t usarDatosEnMsg){
	int16_t RSSI[NUM_MUESTRAS + 2] = {0};
	uint8_t j = 0, i = 0, x = 0;

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

	if((MENSAJE_CON_DATOS == 1) && (usarDatosEnMsg == 1)){ // Si el mensaje contiene datos y además queremos usarlos
		RSSI[NUM_MUESTRAS] = extraerMsg_RSSI(bufferRx);
		RSSI[NUM_MUESTRAS + 1] = RSSI[NUM_MUESTRAS]; // El dato se pone 2 veces para darle mayor peso que al resto
		i = NUM_MUESTRAS + 2;
	}
	else{ i = NUM_MUESTRAS; }

	return filtradoDatos(RSSI, i);
}

int16_t getSNR(uint8_t* bufferRx, uint8_t usarDatosEnMsg){

	int16_t SNR[NUM_MUESTRAS + 1] = {0};
	uint8_t j = 0, i = 0, x = 0;

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

	if((MENSAJE_CON_DATOS == 1) && (usarDatosEnMsg == 1)){ // Si el mensaje contiene datos y además queremos usarlos
		SNR[NUM_MUESTRAS] = extraerMsg_SNR(bufferRx);
		SNR[NUM_MUESTRAS + 1] = SNR[NUM_MUESTRAS]; // El dato se pone 2 veces para darle mayor peso que al resto
		i = NUM_MUESTRAS + 2;
	}
	else{ i = NUM_MUESTRAS; }

	return filtradoDatos(SNR, i);
}

uint8_t construirMsg_RSSI_SNR(uint8_t* mensaje, int16_t RSSI, int16_t SNR){ // Devuelve el tamaño del mensaje creado

	return (uint8_t)sprintf((char*)mensaje, "%dY%d", RSSI, SNR);
}

int16_t getDistancia(int16_t RSSI, int16_t SNR){


	return 1;
}


