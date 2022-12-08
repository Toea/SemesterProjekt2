#include <avr/io.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "x10.h"
#include <util/delay.h>



#define  pinHot 0b00000100
#define  pinCold 0b00000001
#define F_CPU 16000000
#define dataSIZE 30

void StartBlow(int hotTime_);
void StartHeat(int blowTime_);

volatile int count = 1;

uint8_t startRecieved = 0;
uint32_t datapakkeRecieved = 0x0000;
uint16_t addressRecieved = 0;
uint16_t commandRecieved = 0x00;
uint16_t encoded = 0;
uint16_t combined = 0;

uint16_t my_adr = 0b00000101;


int main(void)
{
	// ops�t port/pin
	DDRL = 0xFF;
	// 1 pin til cold - PINA & 0b00000100
	// 1 pin til hot  - PINA & 0b00001000
	int blowTime = 0;
	int hotTime = 0;
	int timeMultiplier = 1; // Skal v�re over 0;

	initPort();
	initISR();

	
	startRecieved = 0;
	datapakkeRecieved = 0x0000;
	addressRecieved = 0;
	commandRecieved = 0x00;
	encoded = 0;
	combined = 0;
	
	while (1)
	{
		// Modtag x10
		if (count == (dataSIZE+2))
		{
			recievex10(&addressRecieved, &commandRecieved, &combined, &encoded, &datapakkeRecieved, &count);
		}
		PORTB = addressRecieved;
		
		switch (commandRecieved)
		{
			case 0b0000000011110000:
			blowTime = 16000000 * timeMultiplier;
			break;
			case 0b0000000000001111:
			hotTime = 16000000 * timeMultiplier;
			break;
			default:
			StartHeat(hotTime);
			StartBlow(blowTime);
			break;
		}
	}
	while (1)
	{
	}
}
void StartBlow(int blowTime_)
{
	if (blowTime_)
	{
		PORTL ^= pinCold;
		blowTime_--;
	}
	else
	{
		PORTL &= ~pinCold;
	}
}
void StartHeat(int hotTime_)
{
	if (hotTime_)
	{
		PORTL ^= pinHot;
		hotTime_--;
	}
	else
	{
		PORTL &= !pinHot;
	}
}




ISR(INT0_vect)
{

	_delay_ms(20);
	uint8_t inputPin = (PINC & 0x1);
	uint32_t longInputPin = 0x0000;
	longInputPin|= inputPin;
	
	if(startRecieved != 0b00001110)
	{
		startRecieved |= inputPin;
		startRecieved = startRecieved << 1;
	}
	else
	{
		if(count <= dataSIZE)
		{
			datapakkeRecieved |= longInputPin;
			datapakkeRecieved = (datapakkeRecieved << 1);
			count++;
		}
		else if (count == (dataSIZE+1))
		{
			datapakkeRecieved = datapakkeRecieved << 2;
			startRecieved = 0;
			//PORTB = (datapakkeRecieved);
			count++;
		}

		
	}

}