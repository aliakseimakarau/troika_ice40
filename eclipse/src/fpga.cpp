/*
 * fpga.cpp
 *
 *  Created on: 18.05.2018
 *      Author: ne23kaj2
 */


#include "gpio.h"
#include "fpga.h"
#include "Timer.h"
#include "diag/Trace.h"

#define SPIx	SPI1

extern Timer timer;
extern volatile uint32_t systick;

FPGAClass::FPGAClass(GPIOClass* gpio) {
	this->m_gpio = gpio;
}

bool FPGAClass::uploadFPGA(uint8_t* dataBuffer, uint32_t dlen) {
	if (m_gpio->read(GPIOClass::CDONE)) {
		return false;
	}

	for (uint32_t i=0;i<dlen;i++) {
		for (int j=7;j>=0;j--) {
			m_gpio->clr(GPIOClass::CONF_SCK);
			if (dataBuffer[i] & (1<<j)) {
				m_gpio->set(GPIOClass::CONF_MOSI);
			} else {
				m_gpio->clr(GPIOClass::CONF_MOSI);
			}
			m_gpio->set(GPIOClass::CONF_SCK);
		}
	}

	for (int i=0;i<100;i++) {
		m_gpio->clr(GPIOClass::CONF_SCK);
		m_gpio->set(GPIOClass::CONF_SCK);
	}

	m_gpio->set(GPIOClass::CONF_SS);

	if (m_gpio->read(GPIOClass::CDONE))
		return true;
	else
		return false;
}

void FPGAClass::startConfigure() {
	m_gpio->clr(GPIOClass::CONF_SS); // low = SPI Slave
	m_gpio->clr(GPIOClass::CRESET_B); // reset FPGA
	m_gpio->set(GPIOClass::CONF_SCK);	// set sck high
	timer.sleep(5);
	m_gpio->set(GPIOClass::CRESET_B);
	timer.sleep(5);

	m_gpio->set(GPIOClass::CONF_SS);
	for (int i=0;i<8;i++) {
		m_gpio->clr(GPIOClass::CONF_SCK);
		m_gpio->set(GPIOClass::CONF_SCK);
	}
	m_gpio->clr(GPIOClass::CONF_SS);
}


bool FPGAClass::isConfigured() {
	return m_gpio->read(GPIOClass::CDONE) ? true : false;
}

void FPGAClass::initSPI() {
	SPI_InitTypeDef SPI_InitStructure;

	lowLevelInit();
	SPI_I2S_DeInit(SPIx);

	SPI_StructInit(&SPI_InitStructure);

	/*!< Deselect the FLASH: Chip Select high */
	m_gpio->set(GPIOClass::POW_SS);

	/*!< SPI configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPIx, &SPI_InitStructure);
	SPI_RxFIFOThresholdConfig(SPIx, SPI_RxFIFOThreshold_QF);  // SPI1->CR2 |= SPI_CR2_FRXTH;

	/*!< Enable the sFLASH_SPI  */
	SPI_Cmd(SPIx, ENABLE);
}

void FPGAClass::lowLevelInit(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/*!< Enable the SPI clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/*!< Enable GPIO clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/*!< SPI pins configuration *************************************************/

	/*!< Connect SPI pins to AF5 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_5);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

	/*!< SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*!< SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*!< SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*!< Configure sFLASH Card CS pin in output pushpull mode ********************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

uint32_t FPGAClass::reverse(uint32_t cmd) {
	uint32_t rev = 0x00000000;

	rev |= (cmd & 0xff000000) >> 24;
	rev |= (cmd & 0x00ff0000) >> 8;
	rev |= (cmd & 0x0000ff00) << 8;
	rev |= (cmd & 0x000000ff) << 24;

	return rev;
}

uint8_t FPGAClass::sendByte(uint8_t byte) {
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
		;
	SPI_SendData8(SPIx, byte);
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
		;
	return SPI_ReceiveData8(SPIx);
}

uint8_t FPGAClass::readByte(void) {
	return (sendByte(0x00));
}

void FPGAClass::send(uint32_t cmd) {
	uint32_t rev = reverse(cmd);
	m_gpio->clr(GPIOClass::POW_SS);
	uint8_t* p = (uint8_t*) &rev;
	for (int i = 0; i < 4; i++) {
		sendByte(p[i]);
	}
	m_gpio->set(GPIOClass::POW_SS);
}

void FPGAClass::fastReceiveBlock(uint32_t* data, int n) {
	// reset address pointer
	send(CMD_RESET_ADDRPTR);

	uint32_t rev = reverse(CMD_READ_DATA);
	uint8_t* p = (uint8_t*) &rev;

	m_gpio->clr(GPIOClass::POW_SS);

	// ignore receiving bytes on first command
	for (int i=0;i<4;i++) {
		sendByte(p[i]);
	}

	for (int j=0;j<n;j++) {
		uint8_t* d = (uint8_t*) data;
		for (int i=0;i<4;i++) {
			d[3-i] = sendByte(p[i]);
		}
//		trace_printf("%08x\n", *data);
		data++;
	}
	m_gpio->set(GPIOClass::POW_SS);
}

void FPGAClass::fastSendBlock(uint32_t* data, int n) {
	send(CMD_RESET_ADDRPTR);
	m_gpio->clr(GPIOClass::POW_SS);
	for (int i=0;i<n;i++) {
		uint32_t cmd = CMD_WRITE_DATA | *data++;
		uint8_t* p = (uint8_t*) &cmd;
		for (int i=0;i<4;i++) {
			sendByte(p[3-i]);
		}
	}
	m_gpio->set(GPIOClass::POW_SS);
}

uint32_t FPGAClass::sendReceive(uint32_t cmd) {
	uint32_t rev = reverse(cmd);
	uint32_t rcv = 0x00000000;

	m_gpio->clr(GPIOClass::POW_SS);
	uint8_t* p = (uint8_t*) &rev;
	for (int i = 0; i < 4; i++) {
		sendByte(p[i]);
	}
	m_gpio->set(GPIOClass::POW_SS);
	m_gpio->clr(GPIOClass::POW_SS);
	uint8_t* d = (uint8_t*) &rcv;
	for (int i = 0; i < 4; i++) {
		d[i] = readByte();
	}
	m_gpio->set(GPIOClass::POW_SS);


	rcv = reverse(rcv);
//    printf("sent: %08x received: %08x\n",cmd, rcv);
	return rcv;
}


// write curl mid-state on FPGA and perform curl optionally
void FPGAClass::troikaBlock(uint8_t* data, int times, bool autopadding) {
	uint32_t dout[27]={0};

	send(CMD_WRITE_FLAGS | FLAG_RESET);
	uint8_t* p = data;
	for (int i=0;i<27;i++) {
		for (int j=0;j<9;j++) {
			dout[i] |= (*p++) << (j*2);
		}
	}
	fastSendBlock(dout, 27);

	uint32_t cmd = CMD_WRITE_FLAGS | FLAG_START;

	if (autopadding)
		cmd |= FLAG_AUTO_PADDING_ENABLE;
	if (times)
		cmd |= (times & 0xff) << 8;

	send(cmd);
	while (sendReceive(CMD_READ_FLAGS) & FLAG_RUNNING) ;
}

void FPGAClass::troikaHash243(uint8_t* data, int times) {
	troikaInit();
	troikaBlock(data, times, true);
}

void FPGAClass::troikaDigest(uint32_t* hash, int u32len) {
	fastReceiveBlock(hash, u32len);
}

void FPGAClass::troikaInit() {
	send(CMD_WRITE_FLAGS | FLAG_RESET);
}

