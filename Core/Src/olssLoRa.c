#include "olssLoRa.h"

void EncenderLEDuC(void){
	HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_RESET);
}

void ApagarLEDuC(void){
	HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, GPIO_PIN_SET);
}

void ToggleLEDuC(void){
	HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
}

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
	    if ((datos[0] >= datos[1] && datos[0] <= datos[2]) || (datos[0] <= datos[1] && datos[0] >= datos[2])){
	    	return datos[0]; }
	    if ((datos[1] >= datos[0] && datos[1] <= datos[2]) || (datos[1] <= datos[0] && datos[1] >= datos[2])){
	    	return datos[1]; }
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

int16_t getRSSI(uint8_t* bufferRx, uint8_t hayDatosEnMsg){

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
			if(bufferRx[i] == '-'){	RSSI[x] = RSSI[x] * (-1); }
		}
		j = 0;
		while(bufferRx[i] != '\r'){i++; }
		i++;
	}

	if((MENSAJE_CON_DATOS == 1) && (hayDatosEnMsg == 1)){ // Si el mensaje contiene datos y además queremos usarlos
		RSSI[NUM_MUESTRAS] = extraerMsg_RSSI(bufferRx);
		RSSI[NUM_MUESTRAS + 1] = RSSI[NUM_MUESTRAS]; // El dato se pone 2 veces para darle mayor peso que al resto
		i = NUM_MUESTRAS + 2;
	}
	else{ i = NUM_MUESTRAS; }

	return filtradoDatos(RSSI, i);
}

int16_t getSNR(uint8_t* bufferRx, uint8_t hayDatosEnMsg){

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

	if((MENSAJE_CON_DATOS == 1) && (hayDatosEnMsg == 1)){ // Si el mensaje contiene datos y además queremos usarlos
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

void getDistancia(int16_t* vectorDistancia, int16_t RSSI, int16_t SNR, float A, float n){
	// vectorDistancia[0] = Distancia calculada
	// vectorDistancia[1] = Límite inferior de posible distancia
	// vectorDistancia[2] = Límite superior de posible distancia
	// A: Valor de referencia. RSSI a 1m de distancia
	// n: Exponente de pérdida

	// Si el SNR es muy bajo, o extremadamente alto, se considera que la medida no es fiable
    if ((SNR < SNR_MIN) || (SNR > SNR_MAX)) {
        vectorDistancia[0] = -9999; // Devolvemos un valor negativo de distancia para indicar que la medida no es válida
        vectorDistancia[1] = -9999;
        vectorDistancia[2] = -9999;
    }
    else{
        // Cálculo de la distancia usando la fórmula: d = 10^(-(RSSI - A) / 10n)
        float exponente = -((float)RSSI - (float)A) / (10.0f * (float)n);
        float potencia = powf(10.0f, exponente);

        vectorDistancia[0] = (int16_t)roundf(potencia); // Redondeamos al entero más cercano en metros

        if(vectorDistancia[0] < 15){ // Si nos encontramos muy cerca, es un caso especial ya que hay más error
        	vectorDistancia[1] = 0;
        	vectorDistancia[2] = 15;
        }
        else{
            // Cálculo de la sensibilidad usando la derivada de la ecuación anterior
            float ln10 = logf(10.0f); // ln(10)
            float factor = -1.0f / (10.0f * n); // -1/(10n)
            int16_t sensibilidad = (int16_t)ceilf(fabsf(potencia) * fabsf(ln10) * fabsf(factor)); // Redondeo al mayor

            vectorDistancia[1] = vectorDistancia[0] - FACTOR_SEGURIDAD_DISTANCIA * sensibilidad;
            vectorDistancia[2] = vectorDistancia[0] + FACTOR_SEGURIDAD_DISTANCIA * sensibilidad;
        }
    }
}

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

void initDisplay(I2C_LCD_HandleTypeDef* lcd1, I2C_HandleTypeDef* hi2c1){
	lcd1->hi2c = hi2c1; // "Handler"
	lcd1->address = 0x4E; // Dirección
	lcd_init(lcd1); // Inicializción a través de la función de la librería
}

void putStringDisplay(I2C_LCD_HandleTypeDef* lcd, char* linea1, char* linea2){
    lcd_clear(lcd);
    lcd_puts(lcd, linea1);
    lcd_gotoxy(lcd, 0, 1);
    lcd_puts(lcd, linea2);
}

void floatToChar_n(char* texto, uint8_t tamTexto, float num){
	// Esta función esta hecha para funcionar únicamente con numeros <10 y con 2 decimales (para el parámetro "n")
	uint8_t cifra1, cifra2, cifra3;

	cifra1 = (uint8_t)(num);
	cifra2 = (uint8_t)((float)(num*10) - (cifra1*10));
	cifra3 = (uint8_t)((float)(num*100) - (cifra1*100) - (cifra2*10));
    snprintf(texto, tamTexto, "Param. n: %d.%d%d", cifra1, cifra2, cifra3);
}

void putDataDisplay(I2C_LCD_HandleTypeDef* lcd, int16_t* vectorDistancia, int16_t RSSI, int16_t SNR){

    char linea1[17];
    char linea2[17];
    int16_t RSSIseguro = 0;
    int16_t SNRseguro = 0;

    // Nos aseguramos de que el RSSI no supere los valores límites
    if(RSSI > 9999){ RSSIseguro = 9999; }
    else if(RSSI < -999){ RSSIseguro = -999; }
    else{RSSIseguro = RSSI; }

    // Nos aseguramos de que el SNR no supere los valores límites
    if(SNR > 9999){ SNRseguro = 9999; }
    else if(SNR < -999){ SNRseguro = -999; }
    else{SNRseguro = SNR; }

    snprintf(linea1, sizeof(linea1), "%d/%d/%d", vectorDistancia[0], RSSIseguro, SNRseguro);
    snprintf(linea2, sizeof(linea2), "%d-%d m", vectorDistancia[1], vectorDistancia[2]);

    putStringDisplay(lcd, linea1, linea2);
}

void calibrarParametros(UART_HandleTypeDef *huart, I2C_LCD_HandleTypeDef* lcd1, float* parametros){

	// Para la calibración, primero la ANTENA debe estar lista

	char linea1[17];
	char linea2[17];

	uint8_t mensajeInicial[] = {"HOLA"}; // Primer mensaje que manda el emisor. Sin datos del RSSI o SNR en él
	uint8_t recibido[TAM_RX_BUFFER] = {0}; // Buffer de recepción de datos
    char nIntentos[17];
	uint8_t intentos = 0;

	HAL_Delay(1000); // Mostramos los parámetros por defecto
    snprintf(linea1, sizeof(linea1), "Param. A: %d", PARAM_A);
    floatToChar_n(linea2, sizeof(linea2), PARAM_n);
	putStringDisplay(lcd1, linea1, linea2);
	HAL_Delay(4000); // Preguntamos si se quiere calibrar o usar los parámetros por defecto
	putStringDisplay(lcd1, "Pulse 1s: NEXT", "Pulse 2s: CONFIG");
	// Comprobamos elección del usuario
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != 0){} // Esperamos a que se pulse el botón "KEY"
	HAL_Delay(2000);
	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != 0){ // Se usan parámetros por defecto
		parametros[0] = PARAM_A;
		parametros[1] = PARAM_n;
	}
	else{ // Se calibran los parámetros manualmente
		// Comenzamos configurando el parámetro "A" a 1m de distancia
		putStringDisplay(lcd1, "Aleje los nodos", "1m y pulse KEY");
		ApagarLEDuC();
		HAL_Delay(2000); // Evitamos detección erróna del botón
		while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != 0){} // Esperamos a que se pulse el botón "KEY"
		putStringDisplay(lcd1, "   Obteniendo   ", "    datos...   ");
		EncenderLEDuC(); // Indicamos que se ha detectado la pulsación
		intentos = 1;
		HAL_Delay(250);
		ApagarLEDuC();
		SendLoRaMS(huart, mensajeInicial, sizeof(mensajeInicial)-1, 6);
		EncenderLEDuC(); // Indicamos que estamos esperando datos
		while(RecLoRaTIMEOUT(huart, recibido) == 30){
			ApagarLEDuC(); // Indicamos que estamos intentando sincronizarnos con la antena
		    snprintf(nIntentos, sizeof(nIntentos), "Intento: %4d", intentos++); // creamos el mensaje con el número de intentos de reconexión
		    putStringDisplay(lcd1, "  Reconectando  ", nIntentos);
		    SendLoRaMS(huart, mensajeInicial, sizeof(mensajeInicial)-1, 6);
		    EncenderLEDuC();
	    }
		ApagarLEDuC(); // Indicamos que se han recibido los datos de vuelta
		parametros[0] = getRSSI(recibido, 1);

		// Configuramos ahora el parámetro "n" a la distancia definida
	    snprintf(linea2, sizeof(linea2), "%dm y pulse KEY", METROS_CALIBRAR_n);
		putStringDisplay(lcd1, "Aleje los nodos", linea2);
		ApagarLEDuC();
		while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != 0){} // Esperamos a que se pulse el botón "KEY
		putStringDisplay(lcd1, "   Obteniendo   ", "    datos...   ");
		EncenderLEDuC(); // Indicamos que se ha detectado la pulsación
		intentos = 1;
		HAL_Delay(250);
		ApagarLEDuC();
		SendLoRaMS(huart, mensajeInicial, sizeof(mensajeInicial)-1, 6);
		EncenderLEDuC(); // Indicamos que estamos esperando datos
		while(RecLoRaTIMEOUT(huart, recibido) == 30){
			ApagarLEDuC(); // Indicamos que estamos intentando sincronizarnos con la antena
		    snprintf(nIntentos, sizeof(nIntentos), "Intento: %4d", intentos++); // creamos el mensaje con el número de intentos de reconexión
		    putStringDisplay(lcd1, "  Reconectando  ", nIntentos);
		    SendLoRaMS(huart, mensajeInicial, sizeof(mensajeInicial)-1, 6);
		    EncenderLEDuC();
	    }
		ApagarLEDuC(); // Indicamos que se han recibido los datos de vuelta
		parametros[1] = fabs((getRSSI(recibido, 1) - parametros[0])/(float)(10 * log10(METROS_CALIBRAR_n)));
	}
	// Mostramos los parámetros tras la calibración
	int16_t paramFormato = parametros[0]; // Ajustamos el formato para evitar errores
    snprintf(linea1, sizeof(linea1), "Param. A: %d", paramFormato);
    floatToChar_n(linea2, sizeof(linea2), parametros[1]);
	putStringDisplay(lcd1, linea1, linea2);
	HAL_Delay(4000); // Esperamos para que el usuario pueda ver la configuración seleccionada
}

uint8_t configAplicacion1(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1, float* parametros){

	uint8_t errorRYLR998 = 100; // 100 es el estado de inicialización de la variable de gestión de errores de comunicación UART con el módulo RYLR998
	uint8_t es_Emisor = 0; // 0 si es antena, 1 si es emisor

	putStringDisplay(lcd1, "Pulse 1s: ANTENA", "Pulse 2s: EMISOR");
    es_Emisor = setTipoNodo(); // Se determina si el dispositivo es el emisor o la antena

    if(es_Emisor == 1){ // Configuración del emisor
	  putStringDisplay(lcd1, "     EMISOR     ", "configurando...");
	  errorRYLR998 = ConfigLoRaModule(huart1, 5, 9);
	  detectRYLR998error(errorRYLR998); // Comprobamos que no haya errores en la comunicación UART con el RYLR998
	  putStringDisplay(lcd1, "     EMISOR     ", "configure param.");
	  HAL_Delay(1000);
	  calibrarParametros(huart1, lcd1, parametros); // Calibramos los parámetros para el cálculo de la distancia
	  putStringDisplay(lcd1, "     EMISOR     ", "---- LISTO -----");
    }
	else{ // Configuración de la antena
	  errorRYLR998 = ConfigLoRaModule(huart1, 6, 9);
	  detectRYLR998error(errorRYLR998); // Comprobamos que no haya errores en la comunicación UART con el RYLR998
	}

	return es_Emisor;
}

void aplicacionEmisor1(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1, float* parametros, int16_t* vectorDistancia){

	uint8_t mensajeInicial[] = {"HOLA"}; // Primer mensaje que manda el emisor. Sin datos del RSSI o SNR en él
	uint8_t recibido[TAM_RX_BUFFER] = {0}; // Buffer de recepción de datos
	uint8_t intentos = 0;
	int16_t RSSI = 0, SNR = 0;
    char nIntentos[17];

	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != 0){} // Esperamos hasta que se pulse el botón "KEY"
	EncenderLEDuC(); // Indicamos que se ha detectado la pulsación
	intentos = 1;
	HAL_Delay(250);
	ApagarLEDuC();
	putStringDisplay(lcd1, "  Enviando msg  ", "    a ANTENA    ");
	if(SendLoRaMS(huart1, mensajeInicial, sizeof(mensajeInicial)-1, 6) == 0){
	  EncenderLEDuC(); // Indicamos que estamos esperando datos
	  putStringDisplay(lcd1, " Esperando msg  ", "   de ANTENA    ");
	  while(RecLoRaTIMEOUT(huart1, recibido) == 30){
		  ApagarLEDuC(); // Indicamos que estamos intentando sincronizarnos con la antena
		  snprintf(nIntentos, sizeof(nIntentos), "Intento: %4d", intentos++); // creamos el mensaje con el número de intentos de reconexión
		  putStringDisplay(lcd1, "  Reconectando  ", nIntentos);
		  SendLoRaMS(huart1, mensajeInicial, sizeof(mensajeInicial)-1, 6);
		  EncenderLEDuC();
	  }
	  ApagarLEDuC(); // Indicamos que se han recibido los datos de vuelta
	  RSSI = getRSSI(recibido, 1);
	  SNR = getSNR(recibido, 1);
	  getDistancia(vectorDistancia, RSSI, SNR, parametros[0], parametros[1]);
	}
	putDataDisplay(lcd1, vectorDistancia, RSSI, SNR); // Mostramos los datos en el Display
    HAL_Delay(1000); // Para evitar errores de falta de sincronización
}

void aplicacionAntena1(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1){

	uint8_t recibido[TAM_RX_BUFFER] = {0}; // Buffer de recepción de datos
	uint8_t mensajeConDatos[9] = {0}; // Vector para almacenar el mensaje creado con los datos del RSSI y SNR
	uint8_t tamMsgCD = 0; // Tamaño del mensaje que contiene datos de RSSI y SNR
	int16_t RSSI = 0, SNR = 0;

	EncenderLEDuC(); // Indicamos que estamos esperando datos
	while(RecLoRa(huart1, recibido) != 0){}
    ApagarLEDuC(); // Indicamos que los datos han sido recibidos
	RSSI = getRSSI(recibido, 0);
	SNR = getSNR(recibido, 0);
	tamMsgCD = construirMsg_RSSI_SNR(mensajeConDatos, RSSI, SNR);
	HAL_Delay(3000);
	SendLoRaMS(huart1, mensajeConDatos, tamMsgCD, 5);
}

uint8_t extraerIDantena(uint8_t* mensaje){

	uint8_t i = 0;

	while(mensaje[i] != ','){ i++; } // Avanzamos hasta la primera coma
	i++;
	while(mensaje[i] != ','){ i++; } // Avanzamos hasta la segunda coma
	i++;

	// Detectamos si lo que se ha mandado es un mensaje normal, o uno para cambiar la dirección de la antena
	// Los mensajes para cambiar la dirección de la antena comienzan por '#'. Ej: #1 para dirección 1
	if(mensaje[i] == '#'){
		i++;
		return (mensaje[i] - '0');
	}
	else{ return 0; }
}

void menuDeConfig(I2C_LCD_HandleTypeDef* lcd1, uint8_t indice){

	switch(indice){
	case 1:
		putStringDisplay(lcd1, "> ANT.1   ANT.2", "  ANT.3   LISTO");
		break;
	case 2:
		putStringDisplay(lcd1, "  ANT.1 > ANT.2", "  ANT.3   LISTO");
		break;
	case 3:
		putStringDisplay(lcd1, "  ANT.1   ANT.2", "> ANT.3   LISTO");
		break;
	case 4:
		putStringDisplay(lcd1, "  ANT.1   ANT.2", "  ANT.3 > LISTO");
		break;
	default:
		putStringDisplay(lcd1, "  ANT.1   ANT.2", "  ANT.3   LISTO");
		break;
	}
}

int16_t modificarCifrasConBotones(I2C_LCD_HandleTypeDef* lcd1, char nAntena, char nCoord){

	uint8_t cifras[4] = {0};
    char linea1[17];
    char linea2[17];

    snprintf(linea1, sizeof(linea1), "Pos. de ANTENA %c", nAntena);
    snprintf(linea2, sizeof(linea2), "Coord.%c:  0000 m", nCoord);
    putStringDisplay(lcd1, linea1, linea2);
    HAL_Delay(500); // Para evitar fallos por pulsaciones anteriores del botón
	// Pulsando los botones 1, 2, 3 y 4 cambiamos el valor de las cifras de la posición de las antenas
	// Pulsamos el botón 5 aceptamos y acabamos la configuración
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) != 0){ // La configuración acaba al pulsar el botón 5
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0){ // Si pulsamos el botón 1 cambiamos los millares
			if(cifras[0] == 9){ cifras[0] = 0; }
			else{ cifras[0]++; }
		    snprintf(linea2, sizeof(linea2), "Coord.%c:  %c%c%c%c m", nCoord, cifras[0]+'0', cifras[1]+'0', cifras[2]+'0', cifras[3]+'0'); // Se imprimen como %c para evitar warnings del compilador
		    putStringDisplay(lcd1, linea1, linea2);
		    HAL_Delay(250); // Para evitar los rebotes
		}
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == 0){ // Si pulsamos el botón 2 cambiamos las centenas
			if(cifras[1] == 9){ cifras[1] = 0; }
			else{ cifras[1]++; }
			snprintf(linea2, sizeof(linea2), "Coord.%c:  %c%c%c%c m", nCoord, cifras[0]+'0', cifras[1]+'0', cifras[2]+'0', cifras[3]+'0');
		    putStringDisplay(lcd1, linea1, linea2);
		    HAL_Delay(250); // Para evitar los rebotes
		}
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == 0){ // Si pulsamos el botón 3 cambiamos las decenas
			if(cifras[2] == 9){ cifras[2] = 0; }
			else{ cifras[2]++; }
			snprintf(linea2, sizeof(linea2), "Coord.%c:  %c%c%c%c m", nCoord, cifras[0]+'0', cifras[1]+'0', cifras[2]+'0', cifras[3]+'0');
		    putStringDisplay(lcd1, linea1, linea2);
		    HAL_Delay(250); // Para evitar los rebotes
		}
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == 0){ // Si pulsamos el botón 4 cambiamos las unidades
			if(cifras[3] == 9){ cifras[3] = 0; }
			else{ cifras[3]++; }
			snprintf(linea2, sizeof(linea2), "Coord.%c:  %c%c%c%c m", nCoord, cifras[0]+'0', cifras[1]+'0', cifras[2]+'0', cifras[3]+'0');
		    putStringDisplay(lcd1, linea1, linea2);
		    HAL_Delay(250); // Para evitar los rebotes
		}
	}
	HAL_Delay(500);
	// Pasamos las cifras seleccionadas a un único int16_t, que es lo que devuelve la función
	return (cifras[0]*1000 + cifras[1]*100 + cifras[2]*10 + cifras[3]);
}

void configCoordenadas(I2C_LCD_HandleTypeDef* lcd1, int16_t* posNodos2x3){

	// Ponemos las coordenadas de la antena 1
    posNodos2x3[0] = modificarCifrasConBotones(lcd1, '1', 'X');
    posNodos2x3[1] = modificarCifrasConBotones(lcd1, '1', 'Y');

	// Ponemos las coordenadas de la antena 2
    posNodos2x3[2] = modificarCifrasConBotones(lcd1, '2', 'X');
    posNodos2x3[3] = modificarCifrasConBotones(lcd1, '2', 'Y');

	// Ponemos las coordenadas de la antena 3
    posNodos2x3[4] = modificarCifrasConBotones(lcd1, '3', 'X');
    posNodos2x3[5] = modificarCifrasConBotones(lcd1, '3', 'Y');
}

uint8_t configAplicacion2(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1, float* vParametros2x3, int16_t* posNodos2x3){

	uint8_t errorRYLR998 = 100; // 100 es el estado de inicialización de la variable de gestión de errores de comunicación UART con el módulo RYLR998
	uint8_t es_Emisor = 0; // 0 si es antena, 1 si es emisor
	uint8_t indiceMenu = 0, indiceMenuAUX = 1;
	uint8_t mensaje[2] = {0};

	// Configuración inicial de los dispositivos
	putStringDisplay(lcd1, "Pulse 1s: ANTENA", "Pulse 2s: EMISOR");
    es_Emisor = setTipoNodo(); // Se determina si el dispositivo es el emisor o la antena

    if(es_Emisor == 1){ // Configuración del emisor
    	putStringDisplay(lcd1, "     EMISOR     ", "configurando...");
    	errorRYLR998 = ConfigLoRaModule(huart1, 5, 9);
	    detectRYLR998error(errorRYLR998); // Comprobamos que no haya errores en la comunicación UART con el RYLR998
	    menuDeConfig(lcd1, indiceMenuAUX);
	    // Configuramos las antenas
	    while(indiceMenu != 4){
		    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0){ // Si pulsamos el botón 1, pasamos al siguiente elemento del menú
		  	    if(indiceMenuAUX == 4){ indiceMenuAUX = 1; }
			    else{ indiceMenuAUX++; }
		  	    menuDeConfig(lcd1, indiceMenuAUX);
			    HAL_Delay(500);
		    }
		    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == 0){ // Si pulsamos el botón 5, se selecciona la opción
			    indiceMenu = indiceMenuAUX;
			    if(indiceMenu != 4){ // Si se selecciona una antena, se envia la información para configurarla
					putStringDisplay(lcd1, "Configure param.", "de la ANTENA");
					HAL_Delay(1000);
					if(indiceMenu == 1){ calibrarParametros(huart1, lcd1, &vParametros2x3[0]); }
					if(indiceMenu == 2){ calibrarParametros(huart1, lcd1, &vParametros2x3[2]); }
					if(indiceMenu == 3){ calibrarParametros(huart1, lcd1, &vParametros2x3[4]); }
					// Preparamos la nueva dirección de la antena
					mensaje[0] = '#';
			    	mensaje [1] = (indiceMenu + '0');
			    	putStringDisplay(lcd1, "Preparando...", "");
				    SendLoRaMS(huart1, mensaje, 2, 6); // Mandamos a la antena su nueva dirección
				    putStringDisplay(lcd1, "     ANTENA     ", "---- LISTA ----");
				    HAL_Delay(1000);
				    menuDeConfig(lcd1, indiceMenuAUX);
			    }
		    }
	    }
    	configCoordenadas(lcd1, posNodos2x3); // El usuario configura las coordenadas de las antenas
	    putStringDisplay(lcd1, "   APLICACION   ", "---- LISTA -----");
	    HAL_Delay(1000);
	    putStringDisplay(lcd1, "Pulse BOT5 para", "solicitar pos.");
    }
	else{ // Configuración de la antena
	  errorRYLR998 = ConfigLoRaModule(huart1, 6, 9);
	  detectRYLR998error(errorRYLR998); // Comprobamos que no haya errores en la comunicación UART con el RYLR998
	}

	return es_Emisor;
}

float funcionCoste(int16_t* posNodos2x3, int16_t* vDistancia3x3, float x, float y){

	// Función de coste que mide lo mal que una posición (x, y) encaja con las distancias esperadas
    float coste = 0, d_real = 0, d_objetivo = 0, error = 0;
    uint8_t i;

    for(i = 0; i < NUM_ANTENAS; i++) {
        d_real = sqrtf((x - posNodos2x3[i*2])*(x - posNodos2x3[i*2]) + (y - posNodos2x3[i*2+1])*(y - posNodos2x3[i*2+1]));
        d_objetivo = 0.5f * (vDistancia3x3[i*3+1] + vDistancia3x3[i*3+2]);
        error = d_real - d_objetivo;
        // Penalizar si estamos fuera del intervalo de distancias para alguna antena
        if (d_real < vDistancia3x3[i*3+1] || d_real > vDistancia3x3[i*3+2]) {
            error = error * PENALIZACION;
        }
        coste = coste + (error * error);
    }
    return coste;
}

float getRMSEdePos(float x, float y, int16_t* posNodos2x3, int16_t* vDistancia3x3){

	// Calculamos el error cuadrático medio para la posición. Teniendo en cuenta el método de cálculo usado en esta aplicación
    float sumaError = 0, d_real = 0, error1 = 0, error2 = 0;
    uint8_t i;

    for(i = 0; i < NUM_ANTENAS; i++) {
        d_real = sqrtf((x - posNodos2x3[i*2])*(x - posNodos2x3[i*2]) + (y - posNodos2x3[i*2+1])*(y - posNodos2x3[i*2+1]));
        error1 = fabs(d_real - vDistancia3x3[i*3+1]);
        error2 = fabs(d_real - vDistancia3x3[i*3+2]);
        sumaError = sumaError + error1 + error2;
    }
    return sumaError/(NUM_ANTENAS*2); // 2 errores por cada antena

}

void getLocalizacion(int16_t* posNodos2x3, int16_t* vDistancia3x3, int16_t* vLocalizacion3){

	// vLocalizacion3[0]: Coordenada X
	// vLocalizacion3[1]: Coordenada Y
	// vLocalizacion3[2]: ERROR

	float x = X_INICIAL, y = Y_INICIAL;
	float x_mejor = 0, y_mejor = 0;
	float x_siguiente = 0, y_siguiente = 0;
	float coste = 0, coste_mejor = 0;
	float paso = PASO_INICIAL;
	int iteracion = 0;
	uint8_t mejoria = 0; // flag
	int8_t dx, dy;

	while((paso > TOLERANCIA_PASO) && (iteracion < ITERACIONES_MAX)){
		x_mejor = x;
		y_mejor = y;
	    coste_mejor = funcionCoste(posNodos2x3, vDistancia3x3, x, y);
	    mejoria = 0; // REiniciamos la flag que indica que se ha mejorado en la iteración
	    // Probamos 8 direcciones alrededor del punto
	    for(dx = -1; dx <= 1; dx++){
	    	for(dy = -1; dy <= 1; dy++){
	    		if(dx == 0 && dy == 0) continue; // La casilla del centro no cuenta, pues es nuestra posición actual
	                x_siguiente = x + dx * paso;
	                y_siguiente = y + dy * paso;
	                coste = funcionCoste(posNodos2x3, vDistancia3x3, x_siguiente, y_siguiente); // Comprobamos cual es la solución de la función de coste para cada casilla
	                if(coste < coste_mejor){ // Si el coste es mejor (mejor resultado), actualizamos los datos a esa casilla
	                	coste_mejor = coste;
	                    x_mejor = x_siguiente;
	                    y_mejor = y_siguiente;
	                    mejoria = 1; // Indicamos con la flag que ha habido una mejoría en esta iteración
	                }
	            }
	        }
	        if(mejoria == 1){ // Si ha habido mejoría, se actualizan los datos con la casilla más óptima
	            x = x_mejor;
	            y = y_mejor;
	        }
	        else{ // Si no ha habido mejoría en la búsqueda por casillas, reducimos el paso a la mitad para seguir buscando más fino
	            paso = paso * 0.5f;  // Reducir paso si no hay mejora
	        }
	        iteracion++;
	    }

	// Guardamos los datos finales en las posiciones correspondientes del vector
	    vLocalizacion3[0] = (int16_t)x;
	    vLocalizacion3[1] = (int16_t)y;
	    vLocalizacion3[2] = (int16_t)getRMSEdePos(x, y, posNodos2x3, vDistancia3x3);
}

void aplicacionEmisor2(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1, float* vParametros2x3, int16_t* posNodos2x3, int16_t* vDistancia3x3, int16_t* vLocalizacion3){

	uint8_t mensajeInicial[] = {"HOLA"}; // Primer mensaje que manda el emisor. Sin datos del RSSI o SNR en él
	uint8_t recibido[TAM_RX_BUFFER] = {0}; // Buffer de recepción de datos
	uint8_t intentos = 0;
	int16_t RSSI[3] = {0}, SNR[3] = {0};
    char nIntentos[17];
    uint8_t i;
    char texto[17];
    char linea1[17];
    char linea2[17];

	// Esperamos hasta pulsar el botón 5
	while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) != 0){}
	EncenderLEDuC(); // Indicamos que se ha detectado la pulsación
	HAL_Delay(250);
	ApagarLEDuC();

	for(i=0;i<3;i++){
		intentos = 1; // Reiniciamos la cuenta de intentos de reconexión
		// Enviamos mensaje a antena
		snprintf(texto, sizeof(texto), "  a ANTENA %d  ", i+1);
		putStringDisplay(lcd1, "  Enviando msg  ", texto);
		if(SendLoRaMS(huart1, mensajeInicial, sizeof(mensajeInicial)-1, i+1) == 0){
			EncenderLEDuC(); // Indicamos que estamos esperando datos
			snprintf(texto, sizeof(texto), " de ANTENA %d  ", i+1);
			putStringDisplay(lcd1, " Esperando msg  ", texto);
			while(RecLoRaTIMEOUT(huart1, recibido) == 30){
				ApagarLEDuC(); // Indicamos que estamos intentando sincronizarnos con la antena
				snprintf(nIntentos, sizeof(nIntentos), "Intento: %4d", intentos++); // creamos el mensaje con el número de intentos de reconexión
				putStringDisplay(lcd1, "  Reconectando  ", nIntentos);
				SendLoRaMS(huart1, mensajeInicial, sizeof(mensajeInicial)-1, i+1);
				EncenderLEDuC();
			}
			ApagarLEDuC(); // Indicamos que se han recibido los datos de vuelta
			RSSI[i] = getRSSI(recibido, 1);
			SNR[i] = getSNR(recibido, 1);
			getDistancia(&vDistancia3x3[i*3], RSSI[i], SNR[i], vParametros2x3[i*2], vParametros2x3[i*2+1]);
		}
		putDataDisplay(lcd1, &vDistancia3x3[i*3], RSSI[i], SNR[i]); // Mostramos los datos en el Display
		HAL_Delay(1000); // Para evitar errores de falta de sincronización
	}

	getLocalizacion(posNodos2x3, vDistancia3x3, vLocalizacion3); // Obtenemos la localización relativa

	// Mostramos los datos de la localización en el display
	snprintf(linea1, sizeof(linea1), "X%d Y%d", vLocalizacion3[0], vLocalizacion3[1]);
	snprintf(linea2, sizeof(linea2), "ERROR:%d (m)", vLocalizacion3[2]);
    putStringDisplay(lcd1, linea1, linea2);

	HAL_Delay(1000); // Para evitar errores de falta de sincronización
}

void aplicacionAntena2(UART_HandleTypeDef* huart1, I2C_LCD_HandleTypeDef* lcd1){

	uint8_t recibido[TAM_RX_BUFFER] = {0}; // Buffer de recepción de datos
	uint8_t mensajeConDatos[9] = {0}; // Vector para almacenar el mensaje creado con los datos del RSSI y SNR
	uint8_t tamMsgCD = 0; // Tamaño del mensaje que contiene datos de RSSI y SNR
	int16_t RSSI = 0, SNR = 0;
	uint8_t IDantena = 0;
	uint8_t i, errorRYLR998;

	EncenderLEDuC(); // Indicamos que estamos esperando datos
	while(RecLoRa(huart1, recibido) != 0){}
    ApagarLEDuC(); // Indicamos que los datos han sido recibidos
    IDantena = extraerIDantena(recibido);
    if(IDantena == 0){
    	RSSI = getRSSI(recibido, 0);
    	SNR = getSNR(recibido, 0);
    	tamMsgCD = construirMsg_RSSI_SNR(mensajeConDatos, RSSI, SNR);
    	HAL_Delay(3000);
    	SendLoRaMS(huart1, mensajeConDatos, tamMsgCD, 5);
    }
    else{ // Si se detecta un cambio de la dirección de la antena
    	// Indicamos con el led qué ID se ha seleccionado. 2 pulsos de luz por cada unidad del ID
		HAL_Delay(1000);
		for(i=0; i<IDantena;i++){
			EncenderLEDuC();
		    HAL_Delay(250);
			ApagarLEDuC();
			HAL_Delay(250);
			EncenderLEDuC();
			HAL_Delay(250);
			ApagarLEDuC();
			HAL_Delay(750);
		}
		// Cambiamos la dirección al ID seleccionado para la antena
		errorRYLR998 = ConfigLoRaModule(huart1, IDantena, 9);
		detectRYLR998error(errorRYLR998); // Comprobamos que no haya errores en la comunicación UART con el RYLR998
    }
}






