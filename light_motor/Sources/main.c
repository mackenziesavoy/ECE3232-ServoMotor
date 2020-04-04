
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
//LEDs sub functions
typedef enum led_state {
	OFF = 0,
	LED_MAN,
	LED_TRACK,
	LED_MOVE,
	LED_CHARGE,
	LED_SLEEP
} led_state_t;

static inline void ledBlueOn(){
	GPIO_HAL_ClearPinOutput(PTB, 21);
}
static inline void ledBlueOff(){
	GPIO_HAL_SetPinOutput(PTB, 21);
}
static inline void ledRedOn(){
	GPIO_HAL_ClearPinOutput(PTB, 22);
}
static inline void ledRedOff(){
	GPIO_HAL_SetPinOutput(PTB, 22);
}
static inline void ledGreenOn(){
	GPIO_HAL_ClearPinOutput(PTE, 26);
}
static inline void ledGreenOff(){
	GPIO_HAL_SetPinOutput(PTE, 26);
}

void ledUpdate(led_state_t state){
	switch(state){
	case LED_MAN: // Purple
		ledBlueOn();
		ledRedOn();
		ledGreenOff();
		break;
	case LED_TRACK: // Blue
		ledBlueOn();
		ledRedOff();
		ledGreenOff();
		break;
	case LED_MOVE: // Red
		ledRedOn();
		ledBlueOff();
		ledGreenOff();
		break;
	case LED_CHARGE: // Green
		ledBlueOff();
		ledRedOff();
		ledGreenOn();
		break;
	case LED_SLEEP: // Yellow
		ledBlueOff();
		ledRedOn();
		ledGreenOn();
		break;
	case OFF: // BLACK
	default: // Also black
		ledBlueOff();
		ledRedOff();
		ledGreenOff();
	}
}

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
	SIM_HAL_EnableClock(SIM, kSimClockGatePortE);

	//set up our mux switches
	PORT_HAL_SetMuxMode(PORTB, 2, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB, 3, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB, 10, kPortMuxAsGpio);

	//Set up LEDS
	PORT_HAL_SetMuxMode(PORTB, 22, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTB, 21, kPortMuxAsGpio);
	PORT_HAL_SetMuxMode(PORTE, 26, kPortMuxAsGpio);

	//set up our manual toggle switch
	PORT_HAL_SetMuxMode(PORTC, 10, kPortMuxAsGpio);

	//Set up our sleep switch
	PORT_HAL_SetMuxMode(PORTC,5,kPortMuxAsGpio);

	//set the pins DDR
	//MUX
	GPIO_HAL_SetPinDir(PTB, 2, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTB, 3, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTB, 10, kGpioDigitalOutput);
	//Manual Mode Switch
	GPIO_HAL_SetPinDir(PTC, 10, kGpioDigitalInput);
	//Sleep Mode Switch
	GPIO_HAL_SetPinDir(PTC,5,kGpioDigitalInput);

	//LEDs
	ledBlueOff();
	ledGreenOff();
	ledRedOff();
	GPIO_HAL_SetPinDir(PTB, 22, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTB, 21, kGpioDigitalOutput);
	GPIO_HAL_SetPinDir(PTE, 26, kGpioDigitalOutput);

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

void DAC0_Init(){
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



void UART0_PutString(char* string){
	while(*string != 0){
		UART0_PutChar(*string);
		string++;
	}
}

void PIT_Init(){
	SIM_HAL_EnableClock(SIM, kSimClockGatePit0);
	PIT_HAL_Enable(PIT);
	PIT_HAL_SetTimerPeriodByCount(PIT,1,20971520); //clock freq * 1000 ms (1 second)
	PIT_HAL_StartTimer(PIT,1);
}

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
	num = (Val[ind[0]]*ind[0]) + (Val[ind[1]]*ind[1]); //sum for numerator
	pos = num/denom; //this will give a position
	return(pos);
}

float SendToDAC(float position){ // Should be run at a fixed-rate to provide integral control
	position = 3.5f - position; //taking the position and minusing it from the average desired index average
	static float unquantized_output = 2047; // Default initialization of integral to middle position
	unquantized_output += position * 0.125f; // Integrate by multiplying position error by gain
	if(unquantized_output > 0xFFF){
		unquantized_output = 0xFFF; // If the output is greater than 12 bits, force it to 12 bits
	}
	else if(unquantized_output < 0){
		unquantized_output = 0; //If the output is less than 0, force it to 0
	}
	uint16_t output = (uint16_t) unquantized_output; // Typecase our float integral output

	DAC_HAL_SetBuffValue(DAC0,0, output); // Write output to DAC
	return(position);
}

uint16_t SendToDACMan(uint16_t position){
	if(position > 0xFFF){
		position = 0xFFF;
	}

	if(position < 0){
		position = 0;
	}
	DAC_HAL_SetBuffValue(DAC0,0, position); // Write output to DAC
	return(position);
}

static inline void clearUart(){
	char buf[] = {0x1B, '[', '2', 'J', 0x00};
	UART0_PutString(buf);
}

int main(void)
{
	char buf[50];
	uint16_t Values[8];
	float DAC_Sent;
	uint16_t Manual;
	int index[8];
	int flag = 0;
	float pos;
	UART0_Init(); //Initialize UART0 for PUTTY
	ADC1_Init();//Initialize ADC1
	ADC0_Init(); //Initialize ADC0
	PIT_Init(); //Initialize PIT
	DAC0_Init(); //Initialize DAC
	GPIO_Init();//Initialize GPIO
	//reseting the PuTTY terminal
	clearUart();

	//MAIN INFINITE LOOP
	while(1){
		if(GPIO_HAL_ReadPinInput(PTC,5) != 0){ //Check sleep mode first as it will take priority
			ledUpdate(LED_SLEEP);
			if(!flag){
				snprintf(buf, 50, "Going to Sleep...\r\n");
				UART0_PutString(buf);
				flag = 1;
			}
			SendToDACMan(0); //Send the motor to its resting position

		}
		else if(GPIO_HAL_ReadPinInput(PTC,10) == 0){ //If the manual mode is not activated
			flag = 0;
			clearUart();
			snprintf(buf, 50, "Auto Mode\r\n");
			ledUpdate(LED_TRACK);
			UART0_PutString(buf);

			while(1){ // creating infinite loop
				if(PIT_HAL_IsIntPending(PIT,1)){
					Gather_Data(Values);
					selectionSort(Values,index);
					pos = getPosition(Values,index);
					ledUpdate(LED_MOVE);
					DAC_Sent = SendToDAC(pos);
					if(DAC_Sent < 0.25){

						break; //If the error in our position is less than 25%
					}
				//Return to the infinite loop until breakS
				}
				ledUpdate(LED_CHARGE);
				PIT_HAL_ClearIntFlag(PIT,1);
				snprintf(buf, 50, "Motor moving to %d", DAC_Sent);
				UART0_PutString(buf);
			}

		}

		else{
			flag = 0;
			ledUpdate(LED_MAN);
			clearUart();
			snprintf(buf, 50, "Gathering Data...\r\n");
			Manual = ADC1_Convert();
			DAC_Sent = SendToDACMan(Manual);
			PIT_HAL_ClearIntFlag(PIT,1);
			snprintf(buf, 50, "Motor moving to %d", DAC_Sent);

		}
	}
	return -1; //If we get here... something went wrong... plz help
}




