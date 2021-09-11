#ifndef BEEPSG_SN76489
#define BEEPSG_SN76489

#include <iostream>
#include <cstdint>
#include <array>
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
    };
}


#endif // BEEPSG_SN76489