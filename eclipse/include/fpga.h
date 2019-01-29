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

#ifndef FPGA_H_
#define FPGA_H_

#include "gpio.h"

#define FLAG_RUNNING                    (1<<0)

#define FLAG_START                      (1<<0)
#define FLAG_START_PADDING				(1<<1)
#define FLAG_RESET			            (1<<2)
#define FLAG_AUTO_PADDING_ENABLE		(1<<3)

#ifndef CMD_NOP
#define CMD_NOP                         0x00000000
#endif

#define CMD_WRITE_FLAGS                 0x04000000	// "000001|xx"
#define CMD_RESET_ADDRPTR               0x08000000	// "000010|xx"
#define CMD_WRITE_DATA                  0x10000000	// "000100|xx"
#define CMD_READ_FLAGS                  0x84000000	// "100001|xx"
#define CMD_READ_DATA					0xa0000000	// "101000|xx"

class FPGAClass {
private:
	uint8_t	dataBuffer[512];
	GPIOClass* m_gpio;
protected:

	void fastReceiveBlock(uint32_t* data, int n);
	void fastSendBlock(uint32_t* data, int n);
	void lowLevelInit(void);
	void send(uint32_t cmd);
	uint32_t sendReceive(uint32_t cmd);
	uint32_t reverse(uint32_t cmd);

	uint8_t sendByte(uint8_t byte);
	uint8_t readByte(void);

public:
	FPGAClass(GPIOClass* gpio);
	bool uploadFPGA(uint8_t* dataBuffer, uint32_t dlen);
	void startConfigure();
	bool isConfigured();

	void initSPI();

	void troikaBlock(uint8_t* data, int times = 1, bool autopadding = false);
	void troikaInit();

	void troikaHash243(uint8_t* data, int times);
	void troikaDigest(uint32_t* hash, int u32len = 27);
};



#endif /* FPGA_H_ */
