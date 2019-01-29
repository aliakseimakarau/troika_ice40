/*
-- Troika for ICE40-FPGA
--
-- 2018 by Thomas Pototschnig <microengineer18@gmail.com,
-- http://microengineer.eu
-- discord: pmaxuw#8292
-- donations-address:
--     LLEYMHRKXWSPMGCMZFPKKTHSEMYJTNAZXSAYZGQUEXLXEEWPXUNWBFDWESOJVLHQHXOPQEYXGIRBYTLRWHMJAOSHUY
--
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
--
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
-- LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
-- WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWAR
*/

#include "gpio.h"
#include "stm32f30x_gpio.h"


#define GPIOA_LED		GPIO_Pin_0



//#define SPI_GPIO      GPIOB
#define GPIOA_MOSI  GPIO_Pin_7
#define GPIOA_MISO  GPIO_Pin_6
#define GPIOA_SCK  	GPIO_Pin_5
#define GPIOA_SS   	GPIO_Pin_3

#define GPIOB_nSTATUS	GPIO_Pin_4
#define GPIOB_DCK		GPIO_Pin_7
#define GPIOB_DATA0		GPIO_Pin_8
#define GPIOB_nCONFIG	GPIO_Pin_9
#define GPIOA_CONFDONE	GPIO_Pin_2
#define GPIOA_PIDETECT	GPIO_Pin_15

#define GPIOB_DISC		GPIO_Pin_2


#define GPIOB_GSM_SLEEP		GPIO_Pin_0
#define GPIOB_GSM_PWRKEY	GPIO_Pin_1

#define GPIOB_CONF_SS		GPIO_Pin_12
#define GPIOB_CONF_SCK		GPIO_Pin_13
#define GPIOB_CONF_MISO		GPIO_Pin_14
#define GPIOB_CONF_MOSI		GPIO_Pin_15

#define GPIOA_CRESET_B		GPIO_Pin_0
#define GPIOA_CDONE			GPIO_Pin_1
#define GPIOA_GSM_RXD		GPIO_Pin_2
#define GPIOA_GSM_TXD		GPIO_Pin_3
#define GPIOA_POW_SS		GPIO_Pin_4
#define GPIOA_POW_SCK		GPIO_Pin_5
#define GPIOA_POW_MISO		GPIO_Pin_6
#define GPIOA_POW_MOSI		GPIO_Pin_7
#define GPIOA_CLK_OUT		GPIO_Pin_8
#define GPIOA_PLL_HOLD		GPIO_Pin_9


/*
#define SPI_CONF_RCC	RCC_APB1Periph_SPI2
#define SPI_CONF		SPI2
#define SPI_GPIO_RCC	RCC_APB2Periph_GPIOB

#define SPI_POW_RCC		RCC_APB1Perip_SPI1
#define SPI_POW			SPI1
#define SPI_GPIO_RCC	RCC_APB2Periph_GPIOA
*/

void GPIOClass::set(GPIOPin* g) {
	GPIO_WriteBit(g->getGPIO(), g->getPinNr(), Bit_SET);
}

void GPIOClass::clr(GPIOPin* g) {
	GPIO_WriteBit(g->getGPIO(), g->getPinNr(), Bit_RESET);
}

void GPIOClass::set(PinsEnum e) {
	set(getGPIOPin(e));
}

void GPIOClass::clr(PinsEnum e) {
	clr(getGPIOPin(e));
}

GPIOPin* GPIOClass::getGPIOPin(PinsEnum e) {
	return Pins[e];
}

BitAction GPIOClass::read(GPIOPin* g) {
	return GPIO_ReadInputDataBit(g->getGPIO(), g->getPinNr()) ? Bit_SET : Bit_RESET;
}

BitAction GPIOClass::read(PinsEnum e) {
	return read(getGPIOPin(e));
}


GPIOClass::GPIOClass() {

//	Pins[PinsEnum::GSM_SLEEP] = new GPIOPinOut(GPIOB, GPIOB_GSM_SLEEP, Bit_SET);
//	Pins[PinsEnum::GSM_PWRKEY] = new GPIOPinOut(GPIOB, GPIOB_GSM_PWRKEY, Bit_SET);

	Pins[PinsEnum::CRESET_B] = new GPIOPinOut(GPIOA, GPIOA_CRESET_B, Bit_RESET);	// reset FPGA
	Pins[PinsEnum::CDONE] = new GPIOPinIn(GPIOA, GPIOA_CDONE, GPIO_PuPd_NOPULL);

	Pins[PinsEnum::CONF_SS] = new GPIOPinOut(GPIOB, GPIOB_CONF_SS, Bit_RESET);		// low = configure as spi slave
	Pins[PinsEnum::CONF_SCK] = new GPIOPinOut(GPIOB, GPIOB_CONF_SCK, Bit_RESET);
	Pins[PinsEnum::CONF_MOSI] = new GPIOPinOut(GPIOB, GPIOB_CONF_MOSI, Bit_RESET);
	Pins[PinsEnum::CONF_MISO] = new GPIOPinIn(GPIOB, GPIOB_CONF_MISO, GPIO_PuPd_NOPULL);

	Pins[PinsEnum::LED] = new GPIOPinOut(GPIOB, GPIOB_LED, Bit_SET);

	Pins[PinsEnum::HSE_EN] = new GPIOPinOut(GPIOB, GPIOB_HSE_EN, Bit_SET);

//	Pins[PinsEnum::GSM_TXD] = new GPIOPinIn(GPIOA, GPIOA_GSM_TXD, GPIO_PuPd_NOPULL);
//	Pins[PinsEnum::GSM_RXD] = new GPIOPinOut(GPIOA, GPIOA_GSM_RXD, Bit_SET);

	Pins[PinsEnum::POW_SS] = new GPIOPinOut(GPIOA, GPIOA_POW_SS, Bit_SET);
/*	Pins[PinsEnum::POW_SCK] = new GPIOPinAF(GPIOA, GPIOA_POW_SCK, Bit_RESET);
	Pins[PinsEnum::POW_MOSI] = new GPIOPinAF(GPIOA, GPIOA_POW_MOSI, Bit_RESET);
	Pins[PinsEnum::POW_MISO] = new GPIOPinAF(GPIOA, GPIOA_POW_MISO, GPIO_PuPd_NOPULL, Bit_SET);*/

	Pins[PinsEnum::PLL_HOLD] = new GPIOPinOut(GPIOA, GPIOA_PLL_HOLD, Bit_RESET);
//	Pins[PinsEnum::CLK_OUT] = new GPIOPinOut(GPIOA, GPIOA_CLK_OUT, Bit_RESET);
}



void GPIOClass::init() {
	GPIO_InitTypeDef GPIO_InitStruct;
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	for (int i=0;i<PinsEnum::NUM_ELEMENTS;i++) {
		if (!Pins[i])
			continue;

		if (Pins[i]->getInitValue() == Bit_SET) {
			set(Pins[i]);
		} else
			clr(Pins[i]);
	}

	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	for (int i=0;i<PinsEnum::NUM_ELEMENTS;i++) {
		if (!Pins[i])
			continue;
		GPIO_InitStruct.GPIO_Pin = Pins[i]->getPinNr();
		GPIO_InitStruct.GPIO_Mode = Pins[i]->getType();
		GPIO_InitStruct.GPIO_OType = Pins[i]->getOType();
		GPIO_InitStruct.GPIO_PuPd = Pins[i]->getPUP();
		GPIO_Init(Pins[i]->getGPIO(), &GPIO_InitStruct);
	}

}
