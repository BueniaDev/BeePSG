#include "ay38910.h"
using namespace beepsg;
using namespace std;

namespace beepsg
{
    AY38910::AY38910()
    {
	// Formula for calculating the volume table is derived from the formula from the CPC Wiki
	// (https://www.cpcwiki.eu/index.php/PSG) with only slight modifications
	for (int i = 0; i < 16; i++)
	{
	    // ampltiude = max / sqrt(2)^(15 - nn)
	    double volume = (1.0 / pow(sqrt(2), (15 - i)));
	    // Normalize ampltiude value to between 0-8192
	    vol_table[i] = int16_t((volume * 8192) + 0.5);
	}

	// Set vol_table[0] to 0 (i.e. silence)
	vol_table[0] = 0;

	tone_freqs.fill(0);
	tone_enabled.fill(false);
	tone_counters.fill(0);
	ch_volume.fill(0);
	tone_output.fill(false);
    }

    AY38910::~AY38910()
    {

    }

    void AY38910::tone_clock()
    {
	for (int numtone = 0; numtone < 3; numtone++)
	{
	    if (--tone_counters[numtone] <= 0)
	    {
		if (tone_freqs[numtone] == 0)
		{
		    tone_counters[numtone] = 1;
		}
		else
		{
		    tone_counters[numtone] = tone_freqs[numtone];
		}

		tone_output[numtone] = !tone_output[numtone];
	    }
	}
    }

    void AY38910::writereg(int reg, uint8_t data)
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
		cout << "Writing value of " << hex << int(data & 0x1F) << " to noise frequency" << endl;
	    }
	    break;
	    case 0x7:
	    {
		cout << "Channel A tone enabled: " << (testbit(data, 0) ? "Yes" : "No") << endl;
		cout << "Channel B tone enabled: " << (testbit(data, 1) ? "Yes" : "No") << endl;
		cout << "Channel C tone enabled: " << (testbit(data, 2) ? "Yes" : "No") << endl;
		cout << "Channel A noise enabled: " << (testbit(data, 3) ? "Yes" : "No") << endl;
		cout << "Channel B noise enabled: " << (testbit(data, 4) ? "Yes" : "No") << endl;
		cout << "Channel C noise enabled: " << (testbit(data, 5) ? "Yes" : "No") << endl;
		cout << "Port A direction: " << (testbit(data, 6) ? "Output" : "Input") << endl;
		cout << "Port B direction: " << (testbit(data, 7) ? "Output" : "Input") << endl;
		tone_enabled[0] = testbit(data, 0);
		tone_enabled[1] = testbit(data, 1);
		tone_enabled[2] = testbit(data, 2);
	    }
	    break;
	    case 0x8:
	    {
		if (testbit(data, 4))
		{
		    cout << "Using envelope for channel A volume" << endl;
		}

		ch_volume[0] = (data & 0xF);
	    }
	    break;
	    case 0x9:
	    {
		if (testbit(data, 4))
		{
		    cout << "Using envelope for channel B volume" << endl;
		}

		ch_volume[1] = (data & 0xF);
	    }
	    break;
	    case 0xA:
	    {
		if (testbit(data, 4))
		{
		    cout << "Using envelope for channel C volume" << endl;
		}

		ch_volume[2] = (data & 0xF);
	    }
	    break;
	    case 0xB:
	    {
		cout << "Writing value of " << hex << int(data) << " to envelope frequency LSB" << endl;
	    }
	    break;
	    case 0xC:
	    {
		cout << "Writing value of " << hex << int(data & 0xF) << " to envelope frequency MSB" << endl;
	    }
	    break;
	    case 0xD:
	    {
		cout << "Envelope hold: " << (testbit(data, 0) ? "Enabled" : "Disabled") << endl;
		cout << "Envelope alternate: " << (testbit(data, 1) ? "Enabled" : "Disabled") << endl;
		cout << "Envelope attack: " << (testbit(data, 2) ? "Enabled" : "Disabled") << endl;
		cout << "Envelope continue: " << (testbit(data, 3) ? "Enabled" : "Disabled") << endl;
	    }
	    break;
	}
    }

    uint32_t AY38910::get_sample_rate(uint32_t clock_rate)
    {
	return (clock_rate / 8);
    }

    uint32_t AY38910::get_divisor() const
    {
	return 8;
    }

    void AY38910::writeIO(int port, uint8_t data)
    {
	if ((port & 1) == 0)
	{
	    chip_register = (data & 0xF);
	}
	else
	{
	    writereg(chip_register, data);
	}
    }

    void AY38910::clockchip()
    {
	tone_clock();
    }

    array<int16_t, 2> AY38910::get_sample()
    {
	int32_t mixed_sample = 0;

	for (int ch = 0; ch < 3; ch++)
	{
	    int volume = 0;
	    if (tone_enabled[ch] || tone_output[ch])
	    {
		volume = ch_volume[ch];
	    }

	    mixed_sample += vol_table[volume];
	}

	int16_t sample = int16_t(mixed_sample);

	array<int16_t, 2> final_samples = {sample, sample};
	return final_samples;
    }
};