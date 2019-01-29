
#ifndef __gpio_H
#define __gpio_H


#include "main.h"

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

#define GPIOB_HSE_EN		GPIO_Pin_9
#define GPIOB_LED			GPIO_Pin_5

class GPIOPin {
private:
	GPIO_TypeDef *m_gpio;
	uint16_t m_pinnr;
	BitAction m_init;
	GPIOMode_TypeDef m_type;
	GPIOOType_TypeDef m_otype;
	GPIOPuPd_TypeDef m_pup;
public:
	GPIOPin(GPIO_TypeDef* gpio, uint16_t pinnr, GPIOMode_TypeDef type, GPIOOType_TypeDef otype, GPIOPuPd_TypeDef pup, BitAction init = Bit_RESET) {
		m_gpio = gpio;
		m_pinnr = pinnr;
		m_init = init;
		m_type = type;
		m_otype = otype;
		m_pup = pup;
	}
	GPIO_TypeDef* getGPIO() {
		return m_gpio;
	}
	uint16_t getPinNr() {
		return m_pinnr;
	}
	BitAction getInitValue() {
		return m_init;
	}
	GPIOMode_TypeDef getType() {
		return m_type;
	}
	GPIOOType_TypeDef getOType() {
		return m_otype;
	}
	GPIOPuPd_TypeDef getPUP() {
		return m_pup;
	}

};

class GPIOPinIn : public GPIOPin {
public:
	GPIOPinIn(GPIO_TypeDef* gpio, uint16_t pinnr, GPIOPuPd_TypeDef pup) : GPIOPin(gpio, pinnr, GPIO_Mode_IN, GPIO_OType_PP, pup, Bit_RESET) {
	}
};

class GPIOPinOut : public GPIOPin {
public:
	GPIOPinOut(GPIO_TypeDef* gpio, uint16_t pinnr, BitAction init = Bit_RESET) : GPIOPin(gpio, pinnr, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL, init) {
	}
};

class GPIOPinAF : public GPIOPin {
public:
	GPIOPinAF(GPIO_TypeDef* gpio, uint16_t pinnr) : GPIOPin(gpio, pinnr, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_NOPULL, Bit_RESET) {
	}
};

class GPIOClass {
public:
	enum PinsEnum {
		GSM_SLEEP, GSM_PWRKEY, CONF_SS, CONF_SCK, CONF_MISO, CONF_MOSI, CRESET_B, CDONE, GSM_RXD, GSM_TXD, POW_SS, POW_SCK, POW_MISO, POW_MOSI, CLK_OUT, PLL_HOLD, LED, HSE_EN, NUM_ELEMENTS
	};

	GPIOPin* Pins[PinsEnum::NUM_ELEMENTS];

	GPIOClass();
	void init();
	void set(GPIOPin* g);
	void clr(GPIOPin* g);
	void set(PinsEnum e);
	void clr(PinsEnum e);

	BitAction read(GPIOPin* g);
	BitAction read(PinsEnum e);
	GPIOPin* getGPIOPin(PinsEnum e);
};




//void init_gpio(void);


#endif

