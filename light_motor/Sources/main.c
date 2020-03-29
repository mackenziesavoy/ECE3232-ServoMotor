
//By Mackenzie Savoy

#include "fsl_device_registers.h"
#include "MK64F12.h"
#include "fsl_gpio_hal.h"
#include "fsl_port_hal.h"
#include "fsl_uart_hal.h"
#include "fsl_sim_hal.h"
#include "fsl_adc16_hal.h"
#include "fsl_PIT_hal.h"
#include "stdio.h"
#include "float.h"
#include "fsl_dac_hal.h"
#include "stdbool.h"

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
		.resolution = kAdc16ResolutionBitOfSingleEndAs12,
		.clkSrc = kAdc16ClkSrcOfBusClk,
		.asyncClkEnable = 0,
		.highSpeedEnable = 0,
		.longSampleCycleMode = kAdc16LongSampleCycleOf24,
		.hwTriggerEnable = 0,
		.refVoltSrc = kAdc16RefVoltSrcOfVref,
		.continuousConvEnable = 0,
		.dmaEnable = 0
};

const dac_converter_config_t dac0_config = {
		.dacRefVoltSrc = kDacRefVoltSrcOfVref1,
		.lowPowerEnable = 0
};
//ensuring that the dac buffer is disabled
const dac_buffer_config_t dac0_buffer_config = {
		.bufferEnable = 0,
};
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
void GPIO_Init(){
	SIM_HAL_EnableClock(SIM, kSimClockGatePortB);
	SIM_HAL_EnableClock(SIM, kSimClockGatePortC);

	//set up our mux switches
	PORT_HAL_SetMuxMode(PORTB, 2, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB, 3, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB, 10, kPortMuxAsGpio);

	//set up our toggle switch
	PORT_HAL_SetMuxMode(PORTC, 10, kPortMuxAsGpio);

	//set the pins DDR
	GPIO_HAL_SetPinDir(PTB, 2, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTB, 3, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTB, 10, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTC, 10, kGpioDigitalInput);

	//initial values don't matter on PTB2, PTB3, PTB10
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

void DAC0_init(){
	SIM_HAL_EnableClock(SIM,kSimClockGateDac0);
	DAC_HAL_Enable(DAC0);
	DAC_HAL_Init(DAC0);
	DAC_HAL_ConfigBuffer(DAC0,&dac0_buffer_config);
	DAC_HAL_ConfigConverter(DAC0,&dac0_config);

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

void initPIT(){
	SIM_HAL_EnableClock(SIM, kSimClockGatePit0);
	PIT_HAL_Enable(PIT);
	PIT_HAL_SetTimerPeriodByCount(PIT,1,2097152); //clock freq * 100 ms
	PIT_HAL_StartTimer(PIT,1);


}
//now for the project specific code

void Gather_Data(uint16_t values[8] ){
	for(int i = 0; i<8; i++){
		if(i == 0){
			GPIO_HAL_ClearPinOutput(PTB,10);
			GPIO_HAL_ClearPinOutput(PTB,3);
			GPIO_HAL_ClearPinOutput(PTB,2);
		}

		else if(i == 1){
			GPIO_HAL_SetPinOutput(PTB,10);
			GPIO_HAL_ClearPinOutput(PTB,3);
			GPIO_HAL_ClearPinOutput(PTB,2);
		}

		else if(i == 2){
			GPIO_HAL_ClearPinOutput(PTB,10);
			GPIO_HAL_SetPinOutput(PTB,3);
			GPIO_HAL_ClearPinOutput(PTB,2);
		}

		else if(i == 3){
			GPIO_HAL_SetPinOutput(PTB,10);
			GPIO_HAL_SetPinOutput(PTB,3);
			GPIO_HAL_ClearPinOutput(PTB,2);
		}

		else if(i == 4){
			GPIO_HAL_ClearPinOutput(PTB,10);
			GPIO_HAL_ClearPinOutput(PTB,3);
			GPIO_HAL_SetPinOutput(PTB,2);
		}

		else if(i == 5){
			GPIO_HAL_SetPinOutput(PTB,10);
			GPIO_HAL_ClearPinOutput(PTB,3);
			GPIO_HAL_SetPinOutput(PTB,2);
		}

		else if(i == 6){
			GPIO_HAL_ClearPinOutput(PTB,10);
			GPIO_HAL_SetPinOutput(PTB,3);
			GPIO_HAL_SetPinOutput(PTB,2);
		}

		else { // i == 7
			GPIO_HAL_SetPinOutput(PTB,10);
			GPIO_HAL_SetPinOutput(PTB,3);
			GPIO_HAL_SetPinOutput(PTB,2);
		}
		values[i] = ADC0_Convert();
	}
	return;
}

bool inArray(int compare, int index[8]){
	for(int i = 0; i<8; i++){
		if(index[i] == compare){
			return true;
		}
	}
	return false;
}

void selectionSort(uint16_t arr[8], int index[8])
{
	for(int i = 0; i<8; i++){
		index[i] = -1; // Initialize index array to invalid indicies
	}

	for(int i = 0; i<8; i++){
		uint16_t max_val = 0;
		int max_ind = 0;
		for(int j = 0; j<8; j++){
			if(!inArray(j, index)){
				if(arr[j] > max_val){
					max_val = arr[j];
					max_ind = j;
				}
			}
		}
		index[i] = max_ind;
	}
}

float getPosition(uint16_t Val[8], int ind[8]){ //Creating a weighted average
	float denom;
	float num;
	float pos;
	denom = (Val[ind[0]] + Val[ind[1]]); //sum for the denominator
	num = (Val[ind[0]]*ind[0])+ (Val[ind[1]]*ind[1]); //sum for numerator
	pos = num/denom; //this will give a position
	return(pos);
}

void SendToDAC(float position){ // Should be run at a fixed-rate to provide integral control
	char buf[50];
	position = 3.5f - position; //taking the position and minusing it from the average desired index average
	static float unquantized_output = 2047; // Default initialization of integral to middle position
	unquantized_output += position * 0.125f; // Integrate by multiplying position error by gain
	uint16_t output = (uint16_t) unquantized_output; // Typecase our float integral output
	if(output > 0x8FF){
		output = 0x8FF; // Limit output value to 12-bit DAC
	}

	DAC_HAL_SetBuffValue(DAC0,0, output); // Write output to DAC
	sprintf(buf, "Position to go to is: %d\r\n",output );
	UART0_PutString(buf);
}

int main(void)
{


	char buf[50];
	uint16_t Values[] = {1 ,8 ,7 ,5, 4 , 5, 9,14};
	int index[8];
	float pos;
	UART0_Init(); //Initialize UART0 for PUTTY
	ADC1_Init();//Initialize ADC1
	initPIT(); //Initialize PIT
	DAC0_init(); //Initialize DAC

	UART0_PutChar('\r');
	UART0_PutChar('\n');
	for(int i = 0; i<8; i++){
			sprintf(buf, "%d \r\n", Values[i]);
			UART0_PutString(buf);
		}






	while(1){

		selectionSort(Values,index);
		pos = getPosition(Values,index);
		if(PIT_HAL_IsIntPending(PIT,1)){
			SendToDAC(pos);
			PIT_HAL_ClearIntFlag(PIT,1);
		}
	}
	return(-1);

}

