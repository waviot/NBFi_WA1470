#include "xtea.h"

const unsigned long DELTA = 0x9E3779B9;

#define NUM_ITERATIONS 64

void XTEA_Encode(unsigned long * data, unsigned char dataLength, xtea_key_t key)
{
	unsigned char i=0;
	unsigned long x1;
	unsigned long x2;
	unsigned long sum;
	unsigned char iterationCount;

	while(i<dataLength)
	{
		sum = 0;
		x1=*data;
		x2=*(data+1);
		iterationCount = NUM_ITERATIONS;

		while(iterationCount > 0)
		{
			x1 += ((x2<<4 ^ x2>>5) + x2) ^ (sum + *(key+(sum&0x03)));
			sum+=DELTA;
			x2 += ((x1<<4 ^ x1>>5) + x1) ^ (sum + *(key+(sum>>11&0x03)));

			iterationCount--;
		}
		*(data++)=x1;
		*(data++)=x2;
		i+=2;
	}
}


void XTEA_Decode(unsigned long * data, unsigned char dataLength, xtea_key_t key)
{
	unsigned char i=0;
	unsigned long x1;
	unsigned long x2;
	unsigned long sum;
	unsigned char iterations;

	iterations = NUM_ITERATIONS;

	while(i<dataLength)
	{
		sum = DELTA*iterations;
		x1=*data;
		x2=*(data+1);

		while(sum != 0)
		{
			x2 -= ((x1<<4 ^ x1>>5) + x1) ^ (sum + *(key+(sum>>11&0x03)));
			sum-=DELTA;
			x1 -= ((x2<<4 ^ x2>>5) + x2) ^ (sum + *(key+(sum&0x03)));
		}
		*(data++)=x1;
		*(data++)=x2;
		i+=2;
	}
}
