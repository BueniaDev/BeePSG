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

#ifndef BEEPSG_AY8910_H
#define BEEPSG_AY8910_H

#include <iostream>
#include <cstdint>
#include <cmath>
#include <vector>
#include <array>
using namespace std;

namespace beepsg
{
    struct ay_param
    {
	double res_up;
	double res_down;
	int res_count;
	array<double, 32> res_values;
    };

    enum AYChipType : int
    {
	AY8910_Chip = 0,
	YM2149_Chip = 1,
    };

    class AY8910
    {
	public:
	    AY8910();
	    ~AY8910();

	    void init(AYChipType type = AY8910_Chip);

	    uint32_t get_sample_rate(uint32_t clk_rate);
	    void writeIO(int port, uint8_t data);

	    void clock_chip();
	    vector<int32_t> get_samples();

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 1) ? true : false;
	    }

	    uint8_t chip_address = 0;

	    void set_chip_type(AYChipType type);
	    void reset();

	    AYChipType chip_type;

	    void write_reg(uint8_t reg, uint8_t data);

	    int32_t get_sample(int ch);

	    void tone_clock();
	    void noise_clock();
	    void envelope_clock();

	    bool is_active = false;

	    bool is_ym_chip = false;
	    bool zero_is_off = false;

	    array<uint32_t, 3> tone_counters;
	    array<uint32_t, 3> tone_freqs;
	    uint32_t noise_counter;
	    uint32_t noise_freq;
	    uint32_t env_counter;
	    array<bool, 3> is_tone_enabled;
	    array<bool, 3> is_noise_enabled;
	    array<bool, 2> is_output_io;
	    array<bool, 3> tone_outputs;
	    bool noise_flip;
	    bool noise_output;
	    array<bool, 3> is_env_enabled;
	    int env_volume;
	    array<uint32_t, 3> ch_volumes;
	    array<double, 3> res_loads;
	    uint16_t env_freq;

	    bool is_env_hold;
	    bool is_env_alt;
	    bool is_env_att;
	    bool is_env_cont;

	    bool is_env_holding;
	    bool is_env_attack;

	    int env_step = 0;
	    int env_mask = 0;
	    int env_max = 0;

	    uint32_t noise_lfsr;

	    array<array<int32_t, 16>, 3> volume_table;
	    array<array<int32_t, 32>, 3> env_table;

	    ay_param vol_params;
	    ay_param env_params;

	    template<size_t S>
	    void build_single_table(double rl, ay_param &par, bool normalize, array<int32_t, S> &table, bool zero_is_off);
    };
};


#endif // BEEPSG_AY8910_H