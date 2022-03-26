/*
    This file is part of the BeePSG engine.
    Copyright (C) 2022 BueniaDev.

    BeePSG is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BeePSG is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BeePSG.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef BEEPSG_SN76489_H
#define BEEPSG_SN76489_H

#include <iostream>
#include <cstdint>
#include <array>
#include <cmath>
using namespace std;

namespace beepsg
{
    class SN76489
    {
	public:
	    SN76489();
	    ~SN76489();

	    uint32_t get_sample_rate(uint32_t clock_rate);
	    uint32_t get_divisor() const;
	    void config(int noisefb, int lfsrbitwidth);
	    void writeIO(uint8_t data);
	    void writestereo(uint8_t data);
	    void clockchip();
	    array<int32_t, 2> get_sample();

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 1) ? true : false;
	    }

	    int parity(int val)
	    {
		val ^= (val >> 8);
		val ^= (val >> 4);
		val ^= (val >> 2);
		val ^= (val >> 1);
		return (val & 1);
	    }

	    void toneclock();
	    void noiseclock();
	    int16_t generate_sample();

	    int noisefeedback = 0;
	    int noisebitmask = 0;
	    uint16_t lfsr = 0x0000;

	    int latchedregister = 0;

	    array<bool, 4> low_or_high;

	    array<int, 4> volume_regs;
	    array<int16_t, 16> volume_table;

	    array<int, 3> noise_value_table = {0x10, 0x20, 0x40};

	    array<int, 3> tone_regs;
	    array<int, 3> tone_vals;
	    int noise_reg = 0;
	    int noise_val = 0;

	    bool output_toggle = false;
    };
}


#endif // BEEPSG_SN76489_H