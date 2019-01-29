

This is an implementation of the Troika hashing algorithmus on ICE40 FPGAs.

The repository is intended to be used by developers who are considering using Troika in their hardware projects. The board-files are from a PoC which can be used as design example.

It's not a high-performance solution because it needs multiple clock-cycles per hash-round and the implementation is heavily restricted by logic resources and achievable speed of ICE40 FPGAs. But the FPGA is very cheap, the PCB can be done by any cheap chinese manufacturer and soldering is doable with a bit of practice. And it's a lot faster than doing hash calculations only in software if using e.g. auto-padding and/or multi-hash-loops. IOTA library functions can benefit much because 1. it works much with 243Trit vectors and 2. they often do multiple hashing loops (e.g. address generation, signing).

Support for this repository is limited because it's just a PoC and intended for experienced developers. (by the way, it's the same PCB as for the PoWChip PoC - for Troika the GSM/BT/GPS modul is not needed and software for it is not included in the repository).

Some features (FPGA):
- ICE40UP5K ($5 FPGA in QFN package)
- FPGA about 73% utilized
- running at 36MHz (uses internal 48MHz oscillator + PLL; no external oscillator required)
- Data-transfers via SPI
- It needs 55 clock cycles per hash
- It can do auto-padding for 243Trit input size (single Block; in IOTA used a lot)
- It can do multiple loops with auto-padding (this is interesting for e.g. IOTA where often a block has to be hashed multiple times; It reduces transfer-times significantly)
- It supports nested hashing by "push"ing" and "pop"ing the internal Troika state to and from a "Stack" (actually, it's an internal offset for RAM address but it acts like a stack).

PCB:
- ICE40UP5K FPGA
- STM32F302CCTx µC (default DFU Bootloader can be used)
- Pinheader for GSM/GPS/BT module (not used in this application)
- Some LEDs, Buttons
- SWD Debug header

Performance:
- 10k blocks (including SPI transfer times) in 11.6s
- 1k blocks with 27 hash-loops (including SPI transfer times and auto-padding) in 2.6s

This repository contains:
- Radiant project files for ICE40 FPGAs with VHDL source files
- Example Board-Design
- Eclipse project with code for STM32F302

Just a small note ... A synthesized ICE40 Bitstream is included in the STM32-code (bitstream.h)

License (MIT):
```
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
```
