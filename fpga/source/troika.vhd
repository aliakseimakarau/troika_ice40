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

library ieee;
 
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.tables.all;

entity troika is
	generic (
		STACK_SIZE : integer := 4	-- how many states can be "pushed" on "stack"
	);
	port
	(
		clk : in std_logic;
		reset : in std_logic;
		
		spi_data_rx : in std_logic_vector(31 downto 0);
		spi_data_tx : out std_logic_vector(31 downto 0);
		spi_data_rxen : in std_logic;
		spi_data_strobe : out std_logic;
		running : out std_logic;
		found : out std_logic;
		overflow : out std_logic
	);
	
end troika;

architecture impl1 of troika is

signal flag_running : std_logic;
signal flag_start : std_logic;
signal flag_start_padding : std_logic;
signal flag_reset : std_logic;

-- for exchange with the state
signal data_rd : std_logic_vector(17 downto 0);
signal data_wr : std_logic_vector(17 downto 0);
--signal addrptr : unsigned(6 downto 0);


-- state memory
type mem_type is array ((STACK_SIZE*128)-1 downto 0) of std_logic_vector(17 downto 0);
signal mem0 : mem_type;
signal mem1 : mem_type;
signal mem2 : mem_type;

 
-- unknown what this really does ... 
attribute syn_ramstyle: string;
attribute syn_ramstyle of mem0: signal is "rw_check";
attribute syn_ramstyle of mem1: signal is "rw_check";
attribute syn_ramstyle of mem2: signal is "rw_check";

signal tmp_state : std_logic_vector(1457 downto 0);	

signal flag_reset_addr : std_logic;
signal flag_read : std_logic;
signal flag_write : std_logic;
signal flag_auto_padding_enable : std_logic;

signal mem_offset : integer range 0 to (STACK_SIZE+128)-1;

signal numhashs : unsigned(7 downto 0);

begin

	running <= flag_running;
	
	process (clk)
	-- because it looks prettier
		variable spi_cmd : std_logic_vector(5 downto 0);
		
		 
	begin
		if rising_edge(clk) then
			flag_start <= '0';
			flag_start_padding <= '0';
			flag_reset <= '0';

			flag_write <= '0';
			flag_read <= '0';
			flag_reset_addr <= '0';
--			flag_auto_padding_enable <= '0';	-- don't reset autopadding flag ... it's needed for hashing multiple hashs
			spi_data_strobe <= '0';--spi_cmd(5);
-- new spi data received
			if spi_data_rxen = '1' then
				spi_cmd := spi_data_rx(31 downto 26);
				case spi_cmd is
					when "000000" => -- nop (mainly for reading back data)
					when "000001" => -- start / stop
					  flag_start <= spi_data_rx(0);
					  flag_start_padding <= spi_data_rx(1);
					  flag_reset <= spi_data_rx(2);
					  flag_auto_padding_enable <= spi_data_rx(3);
					  numhashs <= unsigned(spi_data_rx(15 downto 8));
					when "000010" =>	-- reset write address
						flag_reset_addr <= '1';
					when "000100" =>	-- write to data buffer
						data_wr <= spi_data_rx(17 downto 0);
						flag_write <= '1';
					when "110000" =>	-- "push" state
						if mem_offset < ((STACK_SIZE-1)*128) then
							spi_data_tx(0) <= '1';
							mem_offset <= mem_offset + 128;
						else
							spi_data_tx(0) <= '0';
						end if;
					when "110001" => 	-- "pop state
						if mem_offset > 128 then
							mem_offset <= mem_offset - 128;
							spi_data_tx(0) <= '1';
						else
							spi_data_tx(0) <= '0';
						end if;
					when "100001" =>	-- read flags
						spi_data_tx(0) <= flag_running;
					when "101000" => -- read digest
						spi_data_tx(17 downto 0) <= data_rd;
						flag_read <= '1';
					when others =>
				end case; 
				spi_data_strobe <= spi_cmd(5);
			end if;
		end if;
	end process;

   process(clk)
    variable state : integer range 0 to 31;

	variable row : std_logic_vector(53 downto 0);
	variable lane : std_logic_vector(53 downto 0);
    variable round : integer range 0 to 31;
	variable parity : std_logic_vector(17 downto 0);
	
	variable idx : integer range 0 to 2047;
	
	variable lI : integer range 0 to 31;
	variable lfsr_idx : integer range 0 to 1023;
	
	variable m3 : integer range 0 to 3;
	variable addrptr : unsigned(7 downto 0);
	variable lfsr : std_logic_vector(31 downto 0);
	
	variable autopadding : std_logic;
	variable hashs : unsigned(7 downto 0);
	variable restart : std_logic;
	
    begin
        if rising_edge(clk) then
            if 1=0 then --reset='1' then
                state := 0;
            else
                case state is 
                    when 0 =>
						if flag_reset_addr = '1' then
							m3 := 0;
							addrptr := (others => '0');
						end if;
						
						flag_running <= '0';
						restart := '0';

-- read from mem						
						case m3 is
							when 0 => data_rd <= mem0(to_integer(addrptr));
							when 1 => data_rd <= mem1(to_integer(addrptr));
							when 2 => data_rd <= mem2(to_integer(addrptr));
							when others =>
						end case;
                        
-- write to mem						
						if flag_write='1' then
							case m3 is
								when 0 => mem0(to_integer(addrptr)) <= data_wr;
								when 1 => mem1(to_integer(addrptr)) <= data_wr;
								when 2 => mem2(to_integer(addrptr)) <= data_wr;
								when others =>
							end case;
						end if;
						
-- increment addresses for RAM						
						if flag_read = '1' or flag_write = '1' then
							m3 := m3 + 1;
							if m3 = 3 then
								m3 :=  0;
								addrptr := addrptr + 1;
							end if;
						end if;
				
-- start clearing 2/3rd of state						
						if flag_reset = '1' then
							lI := mem_offset+9; -- only clear 2/3rd of the state
							state := 5;
                        end if;

-- start hashing						
						if flag_start='1' then
							autopadding := flag_auto_padding_enable ;
							hashs := numhashs;
							state := 8;
						end if;
						
-- start hashing of padding						
						if flag_start_padding='1' then
							hashs := (others => '0');	-- disable repeated hashs for padding
							autopadding := '0';			-- disable autopadding for padding
							state := 6;
						end if;

-- sbox and row permutation 
					when 1 =>
						row(53 downto 36) := rol_row(sbox_lookup(mem2(lI)(17 downto 12)) & sbox_lookup(mem2(lI)(11 downto 6)) & sbox_lookup(mem2(lI)(5 downto 0)), 6*2);
						row(35 downto 18) := rol_row(sbox_lookup(mem1(lI)(17 downto 12)) & sbox_lookup(mem1(lI)(11 downto 6)) & sbox_lookup(mem1(lI)(5 downto 0)), 3*2);
						row(17 downto 0)  := rol_row(sbox_lookup(mem0(lI)(17 downto 12)) & sbox_lookup(mem0(lI)(11 downto 6)) & sbox_lookup(mem0(lI)(5 downto 0)), 0*2);
						tmp_state <= row & tmp_state(1457 downto 54);
						lI := lI + 1;
						if lI = mem_offset+27 then
							state := 2;
						end if;
-- lane permutation - this costs a loooot of logic resources ... 						
					when 2 => 
                        for X in 0 to 8 loop 
                            for Y in 0 to 2 loop
                                for Z in 0 to 26 loop
									idx := Z*27+Y*9+X;
                                    lane(Z*2+1 downto Z*2+0) := tmp_state(idx*2+1 downto idx*2);
                                end loop;
                                lane := rol_lane(lane, lane_table(Y*9+X)*2);
                                -- transpose back
                                for Z in 0 to 26 loop
									idx := Z*27+Y*9+X;
                                    tmp_state(idx*2+1 downto idx*2) <= lane(Z*2+1 downto Z*2+0);
                                end loop;
                            end loop;
                        end loop;
						lI := mem_offset;
						state := 3; 
-- calculate and add parity and add round_constants						
					when 3 =>
						-- calculate parity
						parity := vtrit_add(vtrit_add(rol_row(tmp_state(2*18+17 downto 2*18+0), 2), vtrit_add(rol_row(tmp_state(1*18+17 downto 1*18+0), 2), rol_row(tmp_state(0*18+17 downto 0*18+0), 2))),vtrit_add(ror_row(tmp_state(5*18+17 downto 5*18+0), 2), vtrit_add(ror_row(tmp_state(4*18+17 downto 4*18+0), 2), ror_row(tmp_state(3*18+17 downto 3*18+0), 2)))); 
	
						mem0(lI) <= vtrit_add(tmp_state(17 downto 0), vtrit_add(parity, lfsr(17 downto 0)));	-- add parity and lfsr column 0
						mem1(lI) <= vtrit_add(tmp_state(35 downto 18), parity);	-- add parity column 1
						mem2(lI) <= vtrit_add(tmp_state(53 downto 36), parity);	-- add parity column 2

-- calculate LFSR ... it's faster and needs 30% less logic resources				
						for I in 0 to 8 loop
							lfsr(11*2+1 downto 11*2+0) := trit_sub(lfsr(3*2+1 downto 3*2+0), lfsr(1 downto 0));
                            lfsr := "00" & lfsr(31 downto 2);
						end loop;

						tmp_state <= tmp_state(53 downto 0) & tmp_state(1457 downto 54); -- rotation makes lane-wrap-around automatically
						lI := lI + 1;
						if lI = mem_offset+27 then
							round := round + 1;
							lI := mem_offset;
							if round = 24 then
-- if autopadding enabled, do padding and reset flag
								if autopadding = '1' then	
									autopadding := '0';		
									state := 6;
								else
-- if there was only one hash (or hashs not set) then finished									
									if hashs < 2 then
										state := 0;
									else
-- if not, set autopadding, decrement hash-counter and run again								
										autopadding := flag_auto_padding_enable;
										hashs := hashs - 1;
										restart := '1';
										lI := mem_offset+9;
										state := 5;
									end if;
								end if;
							else
								state := 1;
							end if;
						end if;
-- clear 2/3rd of state						
					when 5 =>
						mem0(lI) <= (others => '0');
						mem1(lI) <= (others => '0');
						mem2(lI) <= (others => '0');
						lI := lI + 1;
						if lI = mem_offset+27 then
							if restart = '1' then
								state := 8;
							else
								state := 0;
							end if;
						end if;
-- create padding block for 243trit input						
					when 6 =>	-- clear rate
						mem0(lI) <= (others => '0');
						mem1(lI) <= (others => '0');
						mem2(lI) <= (others => '0');
						lI := lI + 1;
						if lI = mem_offset+9 then
							lI := mem_offset;
							state := 7;
						end if;
					when 7 => -- add padding (can't be done in state 6 because EBR would be replaced by registers)
						mem0(lI) <= "000000000000000001";
						state := 8;
-- initialize hashing						
					when 8 => 
						flag_running <= '1';
						lI := mem_offset;
						round := 0;
						lfsr := x"001449aa";								
						state := 1;
                    when others =>
                        state := 0;
                end case;
            end if;
        end if;
    end process;

	

end impl1;
