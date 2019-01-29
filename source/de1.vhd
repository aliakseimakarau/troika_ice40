-- IOTA Pearl Diver VHDL Port
--
-- 2018 by Thomas Pototschnig <microengineer18@gmail.com,
-- http://microengineer.eu
-- discord: pmaxuw#8292
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

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity de1 is
	port (
--		clock_8M : in std_logic;
		hold : in std_logic;
--		reset : in std_logic;
		led_found : out std_logic;
		led_overflow : out std_logic;
		led_running : out std_logic;
--		////////////////////////	UART	////////////////////////
		spi1_mosi : in std_logic;
		spi1_sck : in std_logic;
		spi1_ss : in std_logic;
		spi1_miso : out std_logic
	);
end;

architecture impl1 of de1 is

signal nreset : std_logic;


signal pll_clk : std_logic;
signal pll_pre_clk : std_logic;
signal pll_reset : std_logic := '0';
signal pll_locked : std_logic;

signal spi_data_tx : std_logic_vector(31 downto 0);
signal spi_data_rx  : std_logic_vector(31 downto 0);
signal spi_data_rx_en : std_logic;
signal spi_data_strobe : std_logic;

signal HFOSC_CLK : std_logic;

signal reset : std_logic;
		
component HSOSC  
GENERIC( CLKHF_DIV :string :="0b00");
PORT(
        CLKHFEN: IN STD_LOGIC ;
        CLKHFPU: IN STD_LOGIC;
        CLKHF:OUT STD_LOGIC
        );
END COMPONENT;


component pow_pll1
port (
	ref_clk_i : in std_logic;
    outcore_o: out std_logic;
    outglobal_o :out std_logic;
    rst_n_i : in std_logic;
	latch_i : in std_logic
);
end component;


component spi_slave
	port
	(
		clk : in std_logic;
		reset : in std_logic;
		
		mosi : in std_logic;
		miso : out std_logic;
		sck : in std_logic;
		ss : in std_logic;
		data_strobe : in std_logic;
		
		
		data_rd : in std_logic_vector(31 downto 0);
		data_wr : out std_logic_vector(31 downto 0);
		data_wren : out std_logic
	);
end component;


component troika
	port
	(
		clk : in std_logic;
		--		clk_slow : in std_logic;
		reset : in std_logic;
		
		spi_data_rx : in std_logic_vector(31 downto 0);
		spi_data_tx : out std_logic_vector(31 downto 0);
		spi_data_rxen : in std_logic;
		spi_data_strobe : out std_logic;

		found : out std_logic;
		overflow : out std_logic;
		running : out std_logic
	);
end component;
 
begin
	reset <= '1';
	nreset <= not reset;

	u_osc : HSOSC
	    GENERIC MAP(CLKHF_DIV =>"0b00")
	    port map(
	        CLKHFEN  => '1',--nreset,
	        CLKHFPU  => '1',--nreset,
	       CLKHF     => HFOSC_CLK
	);
	pll0 : pow_pll1
		port map (
		ref_clk_i => HFOSC_CLK,
	    outcore_o => open,
		outglobal_o => pll_clk,
	    rst_n_i => '1',
		latch_i => hold
	);

	--gb1: SB_GB port map ( 
	--	USER_SIGNAL_TO_GLOBAL_BUFFER => pll_pre_clk,
	--	GLOBAL_BUFFER_OUTPUT => pll_clk
	--);

	spi1 : spi_slave port map (
		clk => pll_clk,
		reset => nreset,
		
		mosi => spi1_mosi,
		miso => spi1_miso,
		sck => spi1_sck,
		ss => spi1_ss,
		data_strobe => spi_data_strobe,
		
		data_rd => spi_data_tx,
		data_wr => spi_data_rx,
		data_wren => spi_data_rx_en
	);
	
	
	troika0 : troika port map (
		clk => pll_clk,
		--clk_slow => pll_clk,
		reset => nreset,
		
		spi_data_rx => spi_data_rx,
		spi_data_tx => spi_data_tx,
		spi_data_rxen => spi_data_rx_en,
		spi_data_strobe => spi_data_strobe,
		
		found => led_found,
		overflow => led_overflow,
		running => led_running
	);
	

end architecture;
