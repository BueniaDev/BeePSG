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

#include "ay8910.h"
using namespace beepsg;

namespace beepsg
{
    static ay_param ay8910_param =
    {
	800000, 8000000,
	16,
	{ 15950, 15350, 15090, 14760, 14275, 13620, 12890, 11370,
        10600,  8590,  7190,  5985,  4820,  3945,  3017,  2345 }
    };

    static ay_param ym2149_param =
    {
	630, 801,
	16,
	{ 73770, 37586, 27458, 21451, 15864, 12371, 8922,  6796,
	   4763,  3521,  2403,  1737,  1123,   762,  438,   251 }
    };

    static ay_param ym2149_param_env = 
    {
	630, 801,
	32,
	{ 103350, 73770, 52657, 37586, 32125, 27458, 24269, 21451,
	   18447, 15864, 14009, 12371, 10506,  8922,  7787,  6796,
	    5689,  4763,  4095,  3521,  2909,  2403,  2043,  1737,
	    1397,  1123,   925,   762,   578,   438,   332,   251 }
    };

    AY8910::AY8910()
    {

    }

    AY8910::~AY8910()
    {

    }

    void AY8910::set_chip_type(AYChipType type)
    {
	if (chip_type != type)
	{
	    chip_type = type;

	    switch (chip_type)
	    {
		case YM2149_Chip: is_ym_chip = true; break;
		case AY8910_Chip: is_ym_chip = false; break;
		default: is_ym_chip = false; break;
	    }
	}
    }

    // Volume table building function (ported from MAME)
    template<size_t S>
    void AY8910::build_single_table(double rl, ay_param &par, bool normalize, array<int32_t, S> &table, bool zero_is_off)
    {
	double rt;
	double rw;

	double temp[32];

	double min = 10.0;
	double max = 0;

	for (int j = 0; j < par.res_count; j++)
	{
	    rt = (1.0 / par.res_down) + (1.0 / rl);

	    rw = (1.0 / par.res_values[j]);
	    rt += (1.0 / par.res_values[j]);

	    if (!(zero_is_off && (j == 0)))
	    {
		rw += (1.0 / par.res_up);
		rt += (1.0 / par.res_up);
	    }

	    temp[j] = (rw / rt);

	    if (temp[j] < min)
	    {
		min = temp[j];
	    }

	    if (temp[j] > max)
	    {
		max = temp[j];
	    }
	}

	if (normalize)
	{
	    for (int j = 0; j < par.res_count; j++)
	    {
		table[j] = int32_t(0x4000 * ((temp[j] - min) / (max - min)) / 3);
	    }
	}
	else
	{
	    for (int j = 0; j < par.res_count; j++)
	    {
		table[j] = int32_t(0x4000 * temp[j] / 3);
	    }
	}
    }

    void AY8910::reset()
    {
	tone_counters.fill(0);
	tone_freqs.fill(0);
	is_tone_enabled.fill(false);
	is_noise_enabled.fill(false);
	is_output_io.fill(false);
	tone_outputs.fill(false);
	env_volume = 0;
	ch_volumes.fill(0);
	is_env_enabled.fill(false);
	env_freq = 0;

	is_env_hold = false;
	is_env_alt = false;
	is_env_att = false;
	is_env_cont = false;

	is_env_attack = false;

	noise_counter = 0;
	noise_freq = 0;

	env_counter = 0;

	noise_flip = false;
	noise_output = false;
	noise_lfsr = 1;

	vol_params = (is_ym_chip) ? ym2149_param : ay8910_param;
	env_params = (is_ym_chip) ? ym2149_param_env : ay8910_param;
	env_step = (is_ym_chip) ? 2 : 1;
	env_mask = (is_ym_chip) ? 31 : 15;
	env_max = (is_ym_chip) ? 31 : 15;
	zero_is_off = (is_ym_chip) ? false : true;

	res_loads.fill(1000);

	for (int ch = 0; ch < 3; ch++)
	{
	    build_single_table(res_loads[ch], vol_params, true, volume_table[ch], zero_is_off);
	    build_single_table(res_loads[ch], env_params, true, env_table[ch], false);
	}
    }

    void AY8910::init(AYChipType type)
    {
	set_chip_type(type);
	reset();
    }

    uint32_t AY8910::get_sample_rate(uint32_t clk_rate)
    {
	return (clk_rate / 8);
    }

    void AY8910::writeIO(int port, uint8_t data)
    {
	if ((port & 1) == 0)
	{
	    is_active = ((data >> 4) == 0);

	    if (is_active)
	    {
		chip_address = (data & 0xF);
	    }
	    else
	    {
		cout << "Warning: Upper address mismatch" << endl;
	    }
	}
	else
	{
	    if (is_active)
	    {
		write_reg(chip_address, data);
	    }
	}
    }

    void AY8910::write_reg(uint8_t reg, uint8_t data)
    {
	if (reg >= 0x10)
	{
	    return;
	}

	switch (reg)
	{
	    case 0x0:
	    {
		tone_freqs[0] = ((tone_freqs[0] & 0xF00) | data);
	    }
	    break;
	    case 0x1:
	    {
		tone_freqs[0] = ((tone_freqs[0] & 0xFF) | ((data & 0xF) << 8));
	    }
	    break;
	    case 0x2:
	    {
		tone_freqs[1] = ((tone_freqs[1] & 0xF00) | data);
	    }
	    break;
	    case 0x3:
	    {
		tone_freqs[1] = ((tone_freqs[1] & 0xFF) | ((data & 0xF) << 8));
	    }
	    break;
	    case 0x4:
	    {
		tone_freqs[2] = ((tone_freqs[2] & 0xF00) | data);
	    }
	    break;
	    case 0x5:
	    {
		tone_freqs[2] = ((tone_freqs[2] & 0xFF) | ((data & 0xF) << 8));
	    }
	    break;
	    case 0x6:
	    {
		noise_freq = (data & 0x1F);
	    }
	    break;
	    case 0x7:
	    {
		is_tone_enabled[0] = testbit(data, 0);
		is_tone_enabled[1] = testbit(data, 1);
		is_tone_enabled[2] = testbit(data, 2);
		is_noise_enabled[0] = testbit(data, 3);
		is_noise_enabled[1] = testbit(data, 4);
		is_noise_enabled[2] = testbit(data, 5);
		is_output_io[0] = testbit(data, 6);
		is_output_io[1] = testbit(data, 7);
	    }
	    break;
	    case 0x8:
	    {
		is_env_enabled[0] = testbit(data, 4);
		ch_volumes[0] = (data & 0xF);
	    }
	    break;
	    case 0x9:
	    {
		is_env_enabled[1] = testbit(data, 4);
		ch_volumes[1] = (data & 0xF);
	    }
	    break;
	    case 0xA:
	    {
		is_env_enabled[2] = testbit(data, 4);
		ch_volumes[2] = (data & 0xF);
	    }
	    break;
	    case 0xB:
	    {
		env_freq = ((env_freq & 0xFF00) | data);
	    }
	    break;
	    case 0xC:
	    {
		env_freq = ((env_freq & 0xFF) | (data << 8));
	    }
	    break;
	    case 0xD:
	    {
		is_env_cont = testbit(data, 3);
		is_env_att = testbit(data, 2);
		is_env_alt = testbit(data, 1);
		is_env_hold = testbit(data, 0);

		is_env_holding = false;
		is_env_attack = is_env_att;
		env_volume = is_env_attack ? 0 : env_max;
	    }
	    break;
	}
    }

    void AY8910::tone_clock()
    {
	for (int numtone = 0; numtone < 3; numtone++)
	{
	    tone_counters[numtone] += 1;

	    if (tone_counters[numtone] >= tone_freqs[numtone])
	    {
		tone_counters[numtone] = 0;
		tone_outputs[numtone] = !tone_outputs[numtone];
	    }
	}
    }

    void AY8910::noise_clock()
    {
	noise_counter += 1;

	if (noise_counter >= noise_freq)
	{
	    noise_counter = 0;
	    noise_flip = !noise_flip;

	    if (noise_flip)
	    {
		noise_output = !testbit(noise_lfsr, 0);
		noise_lfsr = (((testbit(noise_lfsr, 0) != testbit(noise_lfsr, 3)) << 16) | (noise_lfsr >> 1));
	    }
	}
    }

    void AY8910::envelope_clock()
    {
	if (is_env_holding)
	{
	    return;
	}

	env_counter += env_step;

	if (env_counter >= env_freq)
	{
	    env_counter = 0;

	    if (!is_env_attack)
	    {
		if (env_volume != 0)
		{
		    env_volume -= 1;
		    return;
		}
	    }
	    else
	    {
		if (env_volume != env_max)
		{
		    env_volume += 1;
		    return;
		}
	    }

	    if (!is_env_cont)
	    {
		env_volume = 0;
		is_env_holding = true;
	    }
	    else if (is_env_hold)
	    {
		if (is_env_alt)
		{
		    env_volume ^= env_mask;
		}

		is_env_holding = true;
	    }
	    else if (is_env_alt)
	    {
		is_env_attack = !is_env_attack;
	    }
	    else
	    {
		env_volume = is_env_attack ? 0 : env_max;
	    }
	}
    }

    int32_t AY8910::get_sample(int ch)
    {
	if ((ch < 0) || (ch >= 3))
	{
	    return 0;
	}

	int32_t sample = 0;

	bool is_vol_enabled = ((is_tone_enabled[ch] | tone_outputs[ch]) & (is_noise_enabled[ch] | noise_output));

	if (is_vol_enabled)
	{
	    if (is_env_enabled[ch])
	    {
		sample = env_table[ch][env_volume];
	    }
	    else
	    {
		sample = volume_table[ch][ch_volumes[ch]];
	    }
	}

	return sample;
    }

    void AY8910::clock_chip()
    {
	tone_clock();
	noise_clock();
	envelope_clock();
    }

    vector<int32_t> AY8910::get_samples()
    {
	vector<int32_t> samples;
	samples.push_back(get_sample(0));
	samples.push_back(get_sample(1));
	samples.push_back(get_sample(2));
	return samples;
    }
};