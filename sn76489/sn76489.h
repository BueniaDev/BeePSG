#ifndef BEEPSG_SN76489
#define BEEPSG_SN76489

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
	    void config(int noisefb, int lfsrbitwidth);
	    void writeIO(uint8_t data);
	    void writestereo(uint8_t data);
	    void clockchip();
	    array<int16_t, 2> get_sample();

	private:
	    bool testbit(uint32_t reg, int bit);
	    int parity(int val);

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


#endif // BEEPSG_SN76489