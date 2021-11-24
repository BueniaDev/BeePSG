#ifndef BEEPSG_AY38910
#define BEEPSG_AY38910

#include <iostream>
#include <cstdint>
#include <array>
#include <cmath>
using namespace std;

namespace beepsg
{
    class AY38910
    {
	public:
	    AY38910();
	    ~AY38910();

	    uint32_t get_sample_rate(uint32_t clock_rate);
	    uint32_t get_divisor() const;
	    void writeIO(int port, uint8_t data);
	    void clockchip();
	    array<int16_t, 2> get_sample();

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 1) ? true : false;
	    }

	    int chip_register = 0;
	    void writereg(int reg, uint8_t data);

	    array<uint16_t, 3> tone_freqs;
	    array<bool, 3> tone_enabled;
	    array<int, 3> ch_volume;

	    void tone_clock();

	    array<int, 3> tone_counters;
	    array<bool, 3> tone_output;

	    array<int16_t, 16> vol_table;
    };
};

#endif // BEEPSG_AY38910