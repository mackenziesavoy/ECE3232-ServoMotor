
//By Mackenzie Savoy

#include "fsl_device_registers.h"
#include "MK64F12.h"
#include "fsl_gpio_hal.h"
#include "fsl_port_hal.h"
#include "fsl_uart_hal.h"
#include "fsl_sim_hal.h"
#include "fsl_adc16_hal.h"
#include "stdio.h"
#include "float.h"
#include "fsl_dac_hal.h"


//Configuring the ADC Channel
//Developping the Channel Configuration Param for Channel 1
const adc16_chn_config_t chn1_config = {
		.chnIdx = kAdc16Chn1,
		.convCompletedIntEnable = 0,
		.diffConvEnable = 0
};

//Configuration Settings for the Converter(the first one)
const adc16_converter_config_t adc0_config = {
		.lowPowerEnable = 0,
		.clkDividerMode = kAdc16ClkDividerOf1,
		.longSampleTimeEnable = 1,
		.resolution = kAdc16ResolutionBitOf16,
		.clkSrc = kAdc16ClkSrcOfBusClk,
		.asyncClkEnable = 0,
		.highSpeedEnable = 0,
		.longSampleCycleMode = kAdc16LongSampleCycleOf24,
		.hwTriggerEnable = 0,
		.refVoltSrc = kAdc16RefVoltSrcOfVref,
		.continuousConvEnable = 0,
		.dmaEnable = 0
};
//Configuration Settings for the Converter(the second one)
const adc16_converter_config_t adc1_config = {
		.lowPowerEnable = 0,
		.clkDividerMode = kAdc16ClkDividerOf1,
		.longSampleTimeEnable = 1,
		.resolution = kAdc16ResolutionBitOf16,
		.clkSrc = kAdc16ClkSrcOfBusClk,
		.asyncClkEnable = 0,
		.highSpeedEnable = 0,
		.longSampleCycleMode = kAdc16LongSampleCycleOf24,
		.hwTriggerEnable = 0,
		.refVoltSrc = kAdc16RefVoltSrcOfVref,
		.continuousConvEnable = 0,
		.dmaEnable = 0
};
//defininf the initalization codes
void UART0_Init(){

	//Enables the the SIM for the Port E (Location of UART0)
	SIM_HAL_EnableClock(SIM, kSimClockGatePortB);
	SIM_HAL_EnableClock(SIM, kSimClockGateUart0);

	//Setting up all the pins so that we can use it as UART
	PORT_HAL_SetMuxMode(PORTB, 17 , kPortMuxAlt3 ); //UART0 TX



	//Enabling the UART
	UART_HAL_Init(UART0);
	UART_HAL_EnableTransmitter (UART0);

	//Configuring the UART
	UART_HAL_SetBaudRate(UART0, 20971520, 115200);
}
//PTB 2 3 10 ARE THE MUX SWITCHES
//PTC 10 is toggle switch
void Gpio_Init(){

	SIM_HAL_EnableClock(SIM, kSimClockGatePortB);
	SIM_HAL_EnableClock(SIM, kSimClockGatePortC);

	//set up our mux switches
	PORT_HAL_SetMuxMode(PORTB, 2, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB, 3, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB, 10, kPortMuxAsGpio);

	//set up our toggle switch
	PORT_HAL_SetMuxMode(PORTC,10,kPortMuxAsGpio);

	//make sure the mux switches start at 000 and clear the toggle switch
	GPIO_HAL_ClearPinOutput(PTB, 2);
	GPIO_HAL_ClearPinOutput(PTB, 3);
	GPIO_HAL_ClearPinOutput(PTB, 10);
	GPIO_HAL_ClearPinOutput(PTC, 10);


	//set the pins DDR
	GPIO_HAL_SetPinDir(PTB, 2, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTB, 3, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTB, 10, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTC,10,kGpioDigitalInput);
}


void ADC1_Init(){
	SIM_HAL_EnableClock(SIM, kSimClockGateAdc1); //Enabling the SIM for the ADC0
	ADC16_HAL_Init(ADC1); //Intializing the first ADC block
	ADC16_HAL_ConfigConverter(ADC1, &adc1_config); //Importing the ADC0 Configuration

}

uint16_t ADC1_Convert(){
	ADC16_HAL_ConfigChn(ADC1, 0, &chn1_config); //Bringing in the Channel configuration
	while(!ADC16_HAL_GetChnConvCompletedFlag(ADC1, 0)){ //Wait until the converson is complete
	}
	return (ADC16_HAL_GetChnConvValue(ADC1, 0)); //Get the RAW value
}

void ADC0_Init(){
	SIM_HAL_EnableClock(SIM, kSimClockGateAdc0); //Enabling the SIM for the ADC0
	ADC16_HAL_Init(ADC0); //Intializing the first ADC block
	ADC16_HAL_ConfigConverter(ADC0, &adc0_config); //Importing the ADC0 Configuration

}

uint16_t ADC0_Convert(){
	ADC16_HAL_ConfigChn(ADC0, 0, &chn1_config); //Bringing in the Channel configuration
	while(!ADC16_HAL_GetChnConvCompletedFlag(ADC0, 0)){ //Wait until the converson is complete
	}
	return (ADC16_HAL_GetChnConvValue(ADC0, 0)); //Get the RAW value
}


void UART0_PutChar(uint8_t data){
	while(UART_HAL_GetStatusFlag(UART0,kUartTxDataRegEmpty)==0);
	UART_HAL_Putchar(UART0, data); //send an 8 bit character
}

char UART0_GetChar(){
	uint8_t UART0_Data;
	if(UART_HAL_GetStatusFlag(UART0,kUartRxBuffEmpty)==1)
		return (char)0;
	UART_HAL_Getchar(UART0, &UART0_Data); //where do read the data from?
	return (char)UART0_Data;

}

void UART0_PutString(char* string){

	while(*string != 0){
		UART0_PutChar(*string);
		string++;}
}

//now for the project specific code

void Gather_Data(uint16_t* values, uint16_t* values_copy ){
	int i = 0; //counting variable for loop
	while(i<7){
		if(!(GPIO_HAL_ReadPinOutput(PTB,2)) && !(GPIO_HAL_ReadPinOutput(PTB,3)) && !(GPIO_HAL_ReadPinOutput(PTB,10))){
			*values = (uint16_t)ADC0_Convert;
		}

		if(!(GPIO_HAL_ReadPinOutput(PTB,2)) && !(GPIO_HAL_ReadPinOutput(PTB,3)) && (GPIO_HAL_ReadPinOutput(PTB,10))){
			*values = ADC0_Convert;
		}

		if(!(GPIO_HAL_ReadPinOutput(PTB,2)) && (GPIO_HAL_ReadPinOutput(PTB,3)) && !(GPIO_HAL_ReadPinOutput(PTB,10))){
			*values = ADC0_Convert;
		}

		if(!(GPIO_HAL_ReadPinOutput(PTB,2)) && (GPIO_HAL_ReadPinOutput(PTB,3)) && (GPIO_HAL_ReadPinOutput(PTB,10))){
			*values = ADC0_Convert;
		}

		if((GPIO_HAL_ReadPinOutput(PTB,2)) && !(GPIO_HAL_ReadPinOutput(PTB,3)) && !(GPIO_HAL_ReadPinOutput(PTB,10))){
			*values = ADC0_Convert;
		}

		if((GPIO_HAL_ReadPinOutput(PTB,2)) && !(GPIO_HAL_ReadPinOutput(PTB,3)) && (GPIO_HAL_ReadPinOutput(PTB,10))){
			*values = ADC0_Convert;
		}

		if((GPIO_HAL_ReadPinOutput(PTB,2)) && (GPIO_HAL_ReadPinOutput(PTB,3)) && !(GPIO_HAL_ReadPinOutput(PTB,10))){
			*values = ADC0_Convert;
		}

		if((GPIO_HAL_ReadPinOutput(PTB,2)) && !(GPIO_HAL_ReadPinOutput(PTB,3)) && (GPIO_HAL_ReadPinOutput(PTB,10))){
			*values = ADC0_Convert;
		}
		values++;
		i++;
	}
	values_copy = values;
	return;

}

void Quick_Sort(uint16_t Array[8],int first, int last){

	int i, j, pivot;
	uint16_t temp;
	if(first<last){
		pivot=first;
		i=first;
		j=last;


		while(i<j){
			while(Array[i]<=Array[pivot]&&i<last)
				i++;
			while(Array[j]>Array[pivot])
				j--;
			if(i<j){
				temp=Array[i];
				Array[i]=Array[j];
				Array[j]=temp;
			}
		}


		temp=Array[pivot];
		Array[pivot]=Array[j];
		Array[j]=temp;
		Quick_Sort(Array,first,j-1);
		Quick_Sort(Array,j+1,last);
	}

}

int Get_Position(uint16_t values[8], uint16_t values_copy[8]){
	int id1 = 0;
	while(id1 < 7){
		if(values[7] == values_copy[id1]){
			break;
		}
		id1++;
	}
	return(id1);

}


int main(void)
{

	int index;
	int first = 0;
	int last = 7;
	char buf[50];
	uint16_t Values[] = {1 ,8 ,9 ,5, 4 , 5, 7,14};
	uint16_t Values_Copy[8];
	for(int i = 0;i<8;i++){
		Values_Copy[i] = Values[i];
	}

	UART0_Init(); //Initialize UART0 for PUTTY
	ADC1_Init(); //Initialize ADC1

	UART0_PutChar('\r');
	UART0_PutChar('\n');

	for(int i = 0; i<8; i++){
		sprintf(buf, "\r %d ", Values[i]);
		UART0_PutString(buf);
	}



	Quick_Sort(Values,first,last);

	index = Get_Position(Values,Values_Copy);


	while(1){

		for(int i = 0; i<8; i++){
				sprintf(buf, "\r %d ", Values[i]);
				UART0_PutString(buf);
			}
		for(int i = 0; i<8; i++){
				sprintf(buf, "\r %d ", Values_Copy[i]);
				UART0_PutString(buf);
			}
		sprintf(buf, "\r highest number at: %d ", index);
	}
	return(-1);

}

