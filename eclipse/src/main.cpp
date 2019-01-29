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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diag/Trace.h"

#include "gpio.h"

#include "Timer.h"
#include "fpga.h"
#include "hash.h"
//#include "converter.h"
#include "bitstream.h"


#define min(a,b) ((a<b)?a:b)

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

GPIOClass gpio;
FPGAClass fpga(&gpio);

extern volatile uint32_t systick;

extern "C" uint32_t getSysTick() {
	return systick;
}

extern "C" void delay(uint32_t d) {
	uint32_t start = systick;
	while (systick - start < d + 1)
		;
}

void troikaPrint(uint32_t* data) {
	char line[28]={0};
	char* p = line;
	trace_printf("---------------------------\n");
	for (int i=0;i<81;i++) {
		for (int j=0;j<9;j++) {
			*p++=((*data >> (2*j)) & 0x3)+48;
		}
		if ((i % 3)==2) {
			trace_printf("%s\n", line);
			p = line;
		}
		data++;
	}
}

int main(int argc, char* argv[]) {
	gpio.init();
	SystemInit();
	SystemCoreClockUpdate();

	Timer timer;
	timer.start();

	fpga.initSPI();

	fpga.startConfigure();
	fpga.uploadFPGA((uint8_t*) ice40troika_impl_1_bin, ice40troika_impl_1_bin_len);

	if (fpga.isConfigured()) {
		gpio.clr(GPIOClass::LED);
	}

	SystemCoreClockUpdate();

#ifdef DEBUG
	trace_printf("System clock: %u Hz\n", SystemCoreClock);
#endif

	uint8_t data[243]={0};
	data[0]=1;
	data[242]=2;
	uint32_t s = systick;
#if 0
	// single block
	for (int i=0;i<10000;i++) {
		fpga.troikaBlock(data, 1, true);	// 1 loop, auto-padding enabled
	}
#else
	// multiple hash loops
	for (int i=0;i<1000;i++)
		fpga.troikaHash243(data, 27);	// 27 loops, auto-padding enabled
#endif
	trace_printf("%d\n", systick-s);

	uint32_t hash[81]={0};
	fpga.troikaDigest(hash, 81);
	troikaPrint(hash);

	while (1) {
	}

	return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
