#include "sn76489.h"
using namespace beepsg;
using namespace std;

namespace beepsg
{
    SN76489::SN76489()
    {

    }

    SN76489::~SN76489()
    {

    }

    bool SN76489::testbit(uint32_t reg, int bit)
    {
	return ((reg >> bit) & 1);
    }

    uint32_t SN76489::get_sample_rate(uint32_t clock_rate)
    {
	return (clock_rate / 16);
    }

    void SN76489::config(int noisefb, int lfsrbitwidth)
    {
	cout << "Noise feedback: " << dec << noisefb << endl;
	cout << "LFSR bit width: " << dec << lfsrbitwidth << endl;
    }

    void SN76489::writeIO(uint8_t data)
    {
	cout << "Writing value of " << hex << (int)data << " to SN76489 PSG" << endl;
    }

    void SN76489::writestereo(uint8_t data)
    {
	cout << "Writing value of " << hex << (int)data << " to SN76489 Game Gear stereo port" << endl;
    }

    void SN76489::clockchip()
    {
	return;
    }

    array<int16_t, 2> SN76489::get_sample()
    {
	array<int16_t, 2> samples = {0, 0};
	return samples;
    }
};