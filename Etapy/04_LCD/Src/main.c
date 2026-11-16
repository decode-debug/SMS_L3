/****************************************
 * projekt01: RCC configuration         *
 ****************************************/
#include "modbus_conf.h"
#include "modbus.h"
#include "stm32f10x.h"
#include "lcd_hd44780.h"
#include "delay.h"
#include "misc.h"
#include "main.h"
#include <stdbool.h> 		// true, false
#include <stdio.h>

void RCC_Config(void);
void GPIO_Config(void);
void NVIC_Config(void);
void TIM2_Config(void);
void TIM4_Config(void);
void ADC1_Config(void);
void USART1_Config(void);
void LCD_Refresh(void);

char KB2char(void);

uint8_t *resp;
uint16_t resplen;
MB_RESPONSE_STATE respstate;

// Ustaw adres zapisany na obudowie stanowiska przed uruchomieniem.
#define STATION_ADDRESS 1
#define SETPOINT_CENTI_C 2500
#define BAND_CENTI_C 100
#define OUTPUT_POWER 200  // 200 promili = 20% mocy
#define MODBUS_TIMEOUT_MS 1000

static int16_t t1_centi_c;
static bool station_valid;

static bool read_t1(void) {
	uint8_t request[] = {0x00, 0x00, 0x00, 0x01};
	MB_SendRequest(STATION_ADDRESS, FUN_READ_INPUT_REGISTER, request, sizeof(request));
	respstate = MB_GetResponse(STATION_ADDRESS, FUN_READ_INPUT_REGISTER, &resp, &resplen, MODBUS_TIMEOUT_MS);
	if (respstate != RESPONSE_OK || resplen != 3 || resp[0] != 2) return false;
	t1_centi_c = (int16_t)(((uint16_t)resp[1] << 8) | resp[2]);
	return true;
}

static bool write_output(uint16_t reg, uint16_t value) {
	uint8_t request[] = {(uint8_t)(reg >> 8), (uint8_t)reg,
	                     (uint8_t)(value >> 8), (uint8_t)value};
	unsigned int i;
	MB_SendRequest(STATION_ADDRESS, FUN_WRITE_SINGLE_REGISTER, request, sizeof(request));
	respstate = MB_GetResponse(STATION_ADDRESS, FUN_WRITE_SINGLE_REGISTER, &resp, &resplen, MODBUS_TIMEOUT_MS);
	if (respstate != RESPONSE_OK || resplen != sizeof(request)) return false;
	for (i = 0; i < sizeof(request); ++i) if (resp[i] != request[i]) return false;
	return true;
}

static bool regulate(void) {
	if (t1_centi_c < SETPOINT_CENTI_C - BAND_CENTI_C) {
		if (!write_output(0, 0)) return false; // W1 wylacz przed wlaczeniem G1
		return write_output(4, OUTPUT_POWER);
	}
	if (t1_centi_c > SETPOINT_CENTI_C + BAND_CENTI_C) {
		if (!write_output(4, 0)) return false; // G1 wylacz przed wlaczeniem W1
		return write_output(0, OUTPUT_POWER);
	}
	if (!write_output(4, 0)) return false;
	return write_output(0, 0);
}

int main(void) {
	RCC_Config();		  // konfiguracja RCC
	GPIO_Config();    // konfiguracja GPIO
	LED(LEDALL,LED_OFF);
	NVIC_Config();		// konfiguracja NVIC
	TIM2_Config();		// konfigruacja TIM2
	TIM4_Config();		// konfiguracja TIM4
	ADC1_Config();		// konfiguracja ADC1
	USART1_Config();	// konfiguracja USART1
	MB_Config(115200);

	SysTick_Config( 90 ); // 10us
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK_Div8 );
	NVIC_SetPriority(SysTick_IRQn, 0 );

	LCD_Initialize(); // inicjalizacja LCD

	while(1) {
		station_valid = read_t1();
		if (station_valid) station_valid = regulate();
		if (!station_valid) {
			LED(LED1, LED_ON);
			// Proba wylaczenia: bez komunikacji stan wyjsc nie jest znany.
			(void)write_output(4, 0);
			(void)write_output(0, 0);
		} else LED(LED1, LED_OFF);
		LCD_Refresh();
		Delay_ms(1000);
	}
}


void USART1_Config(){
	USART_InitTypeDef USART_InitStruct;

	// Listing ze skryptu: 8 bitow danych, parzystosc, jeden bit stopu.
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_WordLength = USART_WordLength_9b;
	USART_InitStruct.USART_Parity = USART_Parity_Even;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStruct);
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	USART_Cmd(USART1, ENABLE);
}

void Communication_Mode(bool rx, bool tx){
	USART_ITConfig(USART1, USART_IT_RXNE, rx ? ENABLE : DISABLE);
	USART_ITConfig(USART1, USART_IT_TXE, tx ? ENABLE : DISABLE);
}

void Communication_Put(uint8_t ch){
	USART_SendData(USART1, ch);
}

uint8_t Communication_Get(void){
	uint8_t tmp = (uint8_t) USART_ReceiveData(USART1);
	SetCharacterReceived(false);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	return tmp;
}

void Enable50usTimer(void){
	TIM_SetCounter(TIM4, 0);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
}

void Disable50usTimer(void){
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
}

void NVIC_Config(void){
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2);

	// Klawiatura PD6 -> EXTI
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource6);
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Klawiatura PD7 -> EXTI
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource7);
	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Klawiatura PD8 -> EXTI
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource8);
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Klawiatura PD9 -> EXTI
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource9);
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// Klawiatura -- EXTI9_5_IRQn
	NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // Konfiguracja priorytetow TODO
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // Konfiguracja priorytetow TODO
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Przetwornik ADC -- ADC1_2_IRQn
	NVIC_ClearPendingIRQ(ADC1_2_IRQn);
	NVIC_EnableIRQ(ADC1_2_IRQn);
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
}

void RCC_Config(void) {
	ErrorStatus HSEStartUpStatus;                          // zmienna opisujaca rezultat
	// uruchomienia HSE
	// konfigurowanie sygnalow taktujacych
	RCC_DeInit();                                          // reset ustawien RCC
	RCC_HSEConfig(RCC_HSE_ON);                             // wlacz HSE
	HSEStartUpStatus = RCC_WaitForHSEStartUp();            // czekaj na gotowosc HSE
	if(HSEStartUpStatus == SUCCESS) {
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);//
		FLASH_SetLatency(FLASH_Latency_2);                   // zwloka Flasha: 2 takty
		// dla 72MHz
		RCC_HCLKConfig(RCC_SYSCLK_Div1);                     // HCLK = SYSCLK
		RCC_PCLK2Config(RCC_HCLK_Div1);                      // PCLK2 = HCLK
		RCC_PCLK1Config(RCC_HCLK_Div2);                      // PCLK1 = HCLK/2
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // PLLCLK = HSE*9
		// czyli 8MHz * 9 = 72 MHz
		RCC_PLLCmd(ENABLE);                                  // wlacz PLL
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  // czekaj na uruchomienie PLL
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);           // ustaw PLL jako zrodlo
		// sygnalu zegarowego
		while(RCC_GetSYSCLKSource() != 0x08);                // czekaj az PLL bedzie
		// sygnalem zegarowym systemu
		RCC_ADCCLKConfig(RCC_PCLK2_Div6);                    // ADCCLK = PCLK2/6 = 12 MHz

		// konfiguracja sygnalow taktujacych uzywanych peryferii
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);// wlacz taktowanie timera TIM2
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);// wlacz taktowanie timera TIM2
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);// wlacz taktowanie timera TIM4
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE);// wlacz taktowanie portu AFIO
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);// wlacz taktowanie ADC1
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);// wlacz taktowanie portu GPIO A
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);// wlacz taktowanie portu GPIO B
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);// wlacz taktowanie portu GPIO C
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);// wlacz taktowanie portu GPIO D
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);// wlacz taktowanie portu GPIO E
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	}
}

void GPIO_Config(void) {
	// konfigurowanie portow GPIO
	GPIO_InitTypeDef  GPIO_InitStructure;

	// Listing ze skryptu: PA9 = TX, PA10 = RX.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Konfiguracja wejscia analogowego (PB0) -- wejscie analogowe
	// TODO

	// Konfiguracja LEDALL (PB8-PB15) -- wyjscie, push-pull
	GPIO_InitStructure.GPIO_Pin = LEDALL;             		// pin 0
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // wejscie w trybie analogous
	GPIO_Init(GPIOB, &GPIO_InitStructure);                // inicjalizacja portu B

	// Konfiguracja klawiatury (PD10-PD13) -- wyjscie, otwarty dren
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// Konfiguracja klawiatury (PD6-PD9) -- wejscie plywajace
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD, GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13);
}

void LCD_Refresh(void){
	char line[17];
	int t = t1_centi_c / 10; // wyswietlanie z dokladnoscia do 0,1 C
	unsigned int magnitude = (unsigned int)(t < 0 ? -t : t);
	if (station_valid && magnitude <= 999) {
		snprintf(line, sizeof(line), "T:%c%02u.%u Z:%02u.%u  ", t < 0 ? '-' : '+', magnitude / 10, magnitude % 10,
		         (unsigned int)(SETPOINT_CENTI_C / 100), (unsigned int)((SETPOINT_CENTI_C / 10) % 10));
	} else {
		snprintf(line, sizeof(line), "T:ERR   Z:%02u.%u  ", (unsigned int)(SETPOINT_CENTI_C / 100),
		         (unsigned int)((SETPOINT_CENTI_C / 10) % 10));
	}
	LCD_WriteTextXY((unsigned char*)line, 0, 0);
	LCD_WriteTextXY((unsigned char*)"                ", 0, 1);
}

void TIM2_Config(void){
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;
	TIM_TimeBaseStructure.TIM_Period = 5000;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	// konfiguracja kanalu timera
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_Pulse = 1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_ITConfig ( TIM2, TIM_IT_CC2 , ENABLE );
	TIM_Cmd(TIM2, ENABLE);
}

void TIM4_Config(void){
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	// TIM4 = 72 MHz / 72 / 50 = 20 kHz, czyli okres 50 us.
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseStructure.TIM_Period = 50 - 1;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	TIM_Cmd(TIM4, ENABLE);
}

void ADC1_Config(void) {
	ADC_InitTypeDef  ADC_InitStructure;

	ADC_DeInit(ADC1);                                   // reset ustawien ADC1

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  // niezalezne dzialanie ADC 1 i 2
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;       // pomiar pojedynczego kanalu
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // pomiar automatyczny
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2; // start na przerwanie
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // pomiar wyrownany do prawej
	ADC_InitStructure.ADC_NbrOfChannel = 1;             // jeden kanal
	ADC_Init(ADC1, &ADC_InitStructure);                 // inicjalizacja ADC1

	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_41Cycles5); // ADC1, kanal 16,
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

	ADC_Cmd(ADC1, ENABLE);                              // aktywacja ADC1

	ADC_ResetCalibration(ADC1);                         // reset rejestru kalibracji ADC1
	while(ADC_GetResetCalibrationStatus(ADC1));         // oczekiwanie na koniec resetu
	ADC_StartCalibration(ADC1);                         // start kalibracji ADC1
	while(ADC_GetCalibrationStatus(ADC1));              // czekaj na koniec kalibracji
}

char KB2char(void){
	unsigned int GPIO_Pin_row, GPIO_Pin_col, i, j;
	const unsigned char KBkody[16] = {'1','2','3','A',\
			'4','5','6','B',\
			'7','8','9','C',\
			'*','0','#','D'};
	GPIO_SetBits(GPIOD, GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13);
	GPIO_Pin_row = GPIO_Pin_10;
	for(i=0;i<4;++i){
		GPIO_ResetBits(GPIOD, GPIO_Pin_row);
		Delay_ms(5);
		GPIO_Pin_col = GPIO_Pin_6;
		for(j=0;j<4;++j){
			if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_col) == 0){
				GPIO_ResetBits(GPIOD, GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13);
				return KBkody[4*i+j];
			}
			GPIO_Pin_col = GPIO_Pin_col << 1;
		}
		GPIO_SetBits(GPIOD, GPIO_Pin_row);
		GPIO_Pin_row = GPIO_Pin_row << 1;
	}
	GPIO_ResetBits(GPIOD, GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13);
	return 0;
}

void LED(uint16_t led, enum LED_ACTION act) {
	switch(act){
	case LED_ON: GPIO_SetBits(GPIOB, led); break;
	case LED_OFF: GPIO_ResetBits(GPIOB, led); break;
	case LED_TOGGLE: GPIO_WriteBit(GPIOB, led, (GPIO_ReadOutputDataBit(GPIOB, led) == Bit_SET?Bit_RESET:Bit_SET));
	}
}
