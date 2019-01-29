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

package tables is

type const_sbox_table is array ( 0 to 26) of integer;
constant sbox_table : const_sbox_table := (
    0 => 6,
    1 => 25,
    2 => 17,
    3 => 5,
    4 => 15,
    5 => 10,
    6 => 4,
    7 => 20,
    8 => 24,
    9 => 0,
    10 => 1,
    11 => 2,
    12 => 9,
    13 => 22,
    14 => 26,
    15 => 18,
    16 => 16,
    17 => 14,
    18 => 3,
    19 => 13,
    20 => 23,
    21 => 7,
    22 => 11,
    23 => 12,
    24 => 8,
    25 => 21,
    26 => 19
);

type const_row_table is array(0 to 2) of integer;
constant row_table : const_row_table := (
    0 => 0,
    1 => 3,
    2 => 6
);

type const_lane_table is array(0 to 26) of integer;
constant lane_table : const_lane_table := (
    0 => 19,
    1 => 13,
    2 => 21,
    3 => 10,
    4 => 24,
    5 => 15,
    6 => 2,
    7 => 9,
    8 => 3,
    9 => 14,
    10 => 0,
    11 => 6,
    12 => 5,
    13 => 1,
    14 => 25,
    15 => 22,
    16 => 23,
    17 => 20,
    18 => 7,
    19 => 17,
    20 => 26,
    21 => 12,
    22 => 8,
    23 => 18,
    24 => 16,
    25 => 11,
    26 => 4
);

    function sbox_lookup (
        input : std_logic_vector(5 downto 0)
    ) return std_logic_vector;

    function rol_row (
        input : std_logic_vector(17 downto 0);
        n : integer
    ) return std_logic_vector;
    
    function ror_row (
        input : std_logic_vector(17 downto 0);
        n : integer
    ) return std_logic_vector;    

    function rol_lane (
        input : std_logic_vector(53 downto 0);
        n : integer
    ) return std_logic_vector;
	
	function trit_add(
	   a : std_logic_vector(1 downto 0);
	   b : std_logic_vector(1 downto 0)
    ) return std_logic_vector;
    
    function trit_sub(
       a : std_logic_vector(1 downto 0);
       b : std_logic_vector(1 downto 0)
    ) return std_logic_vector;

	function vtrit_add(
		a : std_logic_vector(17 downto 0);
		b : std_logic_vector(17 downto 0)
	) return std_logic_vector;
	
end tables;


package body tables is

    function rol_row (
        input : std_logic_vector(17 downto 0);
        n : integer
    ) return std_logic_vector is
    begin
        return input(17-n downto 0) & input(17 downto 17-n+1);
    end;
    
    function rol_lane (
        input : std_logic_vector(53 downto 0);
        n : integer
    ) return std_logic_vector is
    begin
        return input(53-n downto 0) & input(53 downto 53-n+1);
    end;
    
    function ror_row (
        input : std_logic_vector(17 downto 0);
        n : integer
    ) return std_logic_vector is
    begin
        return input(n-1 downto 0) & input(17 downto n);
    end;
    
    function trit_add(
       a : std_logic_vector(1 downto 0);
       b : std_logic_vector(1 downto 0)
    ) return std_logic_vector is
    variable tmp : std_logic_vector(3 downto 0);
    begin
        tmp := a & b;
        case tmp is
            when "0000" =>  return "00";
            when "0001" =>  return "01";
            when "0010" =>  return "10";
            when "0100" =>  return "01";
            when "0101" =>  return "10";
            when "0110" =>  return "00";
            when "1000" =>  return "10";
            when "1001" =>  return "00";
            when "1010" =>  return "01";
            when others =>  return "00";
        end case;
    end;
	
--	function vtrit_add(
--		a : std_logic_vector(17 downto 0);
--		b : std_logic_vector(17 downto 0)
--	) return std_logic_vector is
--	variable tmp : std_logic_vector(17 downto 0);
--	begin
--		for I in 0 to 8 loop
--			tmp(I*2+1 downto I*2) := trit_add(a(I*2+1 downto I*2), b(I*2+1 downto I*2));
--		end loop;
--		return tmp;
--	end;

-- needs slightly less resources ... (about 3%)
-- and is a little faster
	function vtrit_add(
		a : std_logic_vector(17 downto 0);
		b : std_logic_vector(17 downto 0)
	) return std_logic_vector is
	variable v1 : unsigned(17 downto 0);
	variable v2 : unsigned(17 downto 0);
	variable v3 : unsigned(17 downto 0);
	variable v4 : unsigned(17 downto 0);
	variable v7 : unsigned(17 downto 0);
	variable v8 : unsigned(17 downto 0);
	begin
		v1 := unsigned(a);
		v2 := unsigned(b);
		v3 := v1 xor v2;
		v4 := v1 and v2;
		v7 := shift_right(v1 and resize(x"2aaaa",18), 1) and v2;
		v8 := shift_left(v1 and resize(x"15555",18), 1) and v2;
		v3 := v3 or shift_left(v4 and resize(x"15555",18), 1) or shift_right(v4 and resize(x"2aaaa", 18), 1);
		v3 := v3 and not (v7 or shift_left(v7, 1) or v8 or shift_right(v8, 1));

		return std_logic_vector(v3);
	end;


    function trit_sub(
       a : std_logic_vector(1 downto 0);
       b : std_logic_vector(1 downto 0)
    ) return std_logic_vector is
    variable tmp : std_logic_vector(3 downto 0);
    begin
        tmp := a & b;
        case tmp is
            when "0000" =>  return "00";
            when "0001" =>  return "10";
            when "0010" =>  return "01";
            when "0100" =>  return "01";
            when "0101" =>  return "00";
            when "0110" =>  return "10";
            when "1000" =>  return "10";
            when "1001" =>  return "01";
            when "1010" =>  return "00";
            when others =>  return "00";
        end case;
    end;


    function sbox_lookup (
        input : std_logic_vector(5 downto 0)
    ) return std_logic_vector is
    begin
        case input is
            when "000000" => return "001000";
            when "000001" => return "000000";
            when "000010" => return "000100";
            when "000100" => return "100100";
            when "000101" => return "000001";
            when "000110" => return "011000";
            when "001000" => return "010100";
            when "001001" => return "000010";
            when "001010" => return "101000";
            when "010000" => return "011010";
            when "010001" => return "010000";
            when "010010" => return "010101";
            when "010100" => return "001001";
            when "010101" => return "010110";
            when "010110" => return "100001";
            when "011000" => return "100010";
            when "011001" => return "011001";
            when "011010" => return "000110";
            when "100000" => return "101001";
            when "100001" => return "100000";
            when "100010" => return "100110";
            when "100100" => return "010001";
            when "100101" => return "101010";
            when "100110" => return "000101";
            when "101000" => return "001010";
            when "101001" => return "100101";
            when "101010" => return "010010";
            when others => return "000000";
        end case;
    end;
end tables;
