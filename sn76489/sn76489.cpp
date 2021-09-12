#include "sn76489.h"
using namespace beepsg;
using namespace std;

namespace beepsg
{
    SN76489::SN76489()
    {
	noisefeedback = 9;
	noisebitmask = 15;
	lfsr = 0x8000;

	for (int level = 0; level < 15; level++)	
	{
	    float value = pow(pow(10.0, -0.1), level);
	    volume_table[level] = int16_t((value * 2048) + 0.5);
	}

	volume_table[0xF] = 0;

	volume_regs.fill(0xF);
	tone_regs.fill(0);
	tone_vals.fill(0);
	low_or_high.fill(false);
    }

    SN76489::~SN76489()
    {

    }

    bool SN76489::testbit(uint32_t reg, int bit)
    {
	return ((reg >> bit) & 1);
    }

    int SN76489::parity(int val)
    {
	val ^= (val >> 8);
	val ^= (val >> 4);
	val ^= (val >> 2);
	val ^= (val >> 1);
	return (val & 1);
    }

    void SN76489::toneclock()
    {
	for (int numtone = 0; numtone < 3; numtone++)
	{
	    if (--tone_vals[numtone] <= 0)
	    {
		// If tone register is zero, output is +1
		// (used for sample playback)
		if (tone_regs[numtone] == 0)
		{
		    low_or_high[numtone] = true;
		}
		else
		{
		    low_or_high[numtone] = !low_or_high[numtone];
		}

		tone_vals[numtone] = tone_regs[numtone];
	    }
	}
    }

    void SN76489::noiseclock()
    {
	if (--noise_val <= 0)
	{
	    output_toggle = !output_toggle;

	    if (output_toggle)
	    {
		low_or_high[3] = testbit(lfsr, 0);
		int value_rotated = testbit(noise_reg, 2) ? parity((lfsr & noisefeedback)) : testbit(lfsr, 0);
		lfsr >>= 1;
		lfsr |= (value_rotated << noisebitmask);
	    }

	    int noise_reload = (noise_reg & 0x3);
	    noise_val = (noise_reload == 3) ? tone_regs[2] : noise_value_table[noise_reload];
	}
    }

    int16_t SN76489::generate_sample()
    {
	int16_t sample = 0;

	for (int channel = 0; channel < 4; channel++)
	{
	    int volume = !low_or_high[channel] ? 0xF : volume_regs[channel];
	    sample += volume_table[volume];
	}

	return sample;
    }

    uint32_t SN76489::get_sample_rate(uint32_t clock_rate)
    {
	return (clock_rate / 16);
    }

    uint32_t SN76489::get_divisor() const
    {
	return 16;
    }

    void SN76489::config(int noisefb, int lfsrbitwidth)
    {
	noisefeedback = noisefb;
	noisebitmask = (lfsrbitwidth - 1);
	lfsr = (1 << noisebitmask);
    }

    void SN76489::writeIO(uint8_t data)
    {
	if (testbit(data, 7))
	{
	    int channel = ((data >> 5) & 0x3);
	    bool is_volume_reg = testbit(data, 4);
	    latchedregister = ((is_volume_reg << 2) | channel);
	    
	    if (is_volume_reg)
	    {
		volume_regs[channel] = (data & 0xF);
	    }
	    else
	    {
		if (channel == 3)
		{
		    noise_reg = (data & 0x7);
		}
		else
		{
		    tone_regs[channel] = ((tone_regs[channel] & 0x3F0) | (data & 0xF));
		}
	    }
	}
	else
	{
	    bool is_volume_reg = testbit(latchedregister, 2);
	    int channel = (latchedregister & 0x3);

	    if (is_volume_reg)
	    {
		volume_regs[channel] = (data & 0xF);
	    }
	    else
	    {
		if (channel == 3)
		{
		    noise_reg = (data & 0x7);
		}
		else
		{
		    tone_regs[channel] = (((data & 0x3F) << 4) | (tone_regs[channel] & 0xF));
		}
	    }
	}
    }

    void SN76489::writestereo(uint8_t data)
    {
	cout << "Writing value of " << hex << (int)data << " to SN76489 Game Gear stereo port" << endl;
    }

    void SN76489::clockchip()
    {
	toneclock();
	noiseclock();
    }

    array<int16_t, 2> SN76489::get_sample()
    {
	int16_t left = generate_sample();
	int16_t right = generate_sample();
	array<int16_t, 2> samples = {left, right};
	return samples;
    }
};