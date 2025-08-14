#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define TWO_48 281474976710656.0f
#define TWO_16 65536.0f
#define TEN_MILLION 10000000

typedef uint64_t u64;
typedef uint32_t u32;
typedef __uint128_t u128;

double new_sqrt(double s){
	int exp;
	double a = frexp(s,&exp);
	if (exp & 1){
		a*=2.0f;
		--exp;
	}
	a = ldexp(0.485f+0.485f*a,exp>>1);
	a = 0.5*(a+s/a);
	a = 0.5*(a+s/a);
	return a;
}

u32 bit_by_bit_Q1616_best(u32 N){ // N is actual u32, return type is Q16.16
	u64 remainder = (u64)N<<32;
	u32 result = 0;
	for(u32 pow2 = 1<<(16 + ((31-__builtin_clz(N))>>1)); pow2 > 0; pow2>>=1) {
		u64 temp = (u64)((result<<1) | pow2) * pow2; // could replace * pow2 with << i
		if (temp <= remainder) {
			remainder -= temp;
			result |= pow2;
		}
	}
	return result;
}

u32 bit_by_bit_Q1616_v2(u32 N){ // N is actual u32, return type is Q16.16
	u64 remainder = (u64)N<<32;
	u32 result = 0;
	int largest_possible_bit = 16 + ((31-__builtin_clz(N))>>1);
	for(int i = largest_possible_bit; i>-1; --i) {
		u32 pow2 = 1<<i;
		u64 temp = (u64)((result<<1) | pow2) * pow2; // could replace * pow2 with << i
		if (temp <= remainder) {
			remainder -= temp;
			result |= pow2;
		}
	}
	return result;
}

u32 bit_by_bit_Q1616_branchless(u32 N) {
    u64 remainder = (u64)N << 32;
    u32 result = 0;
    int largest_possible_bit = 16 + ((31 - __builtin_clz(N)) >> 1);
    
    for (int i = largest_possible_bit; i > -1; --i) {
        u32 pow2 = 1 << i;
        u64 temp = (u64)((result << 1) | pow2) <<i;
        
        u32 condition = (temp <= remainder);
        remainder -= temp * condition;  // Only subtract if condition is true
        result |= pow2 * condition;     // Only set bit if condition is true
    }
    return result;
}

u64 bit_by_bit_Q1648(u32 N) { // N is actual u32, return type is Q16.48
    u128 remainder = (u128)N << 96;
    u64 result = 0;
    
    for(int i = 48 + ((31 - __builtin_clz(N))>>1); i > -1; --i) {
        u64 pow2 = (u64)1 << i;
        u128 temp = (u128)((result << 1) + pow2) << i;
        if (temp <= remainder) {
            remainder -= temp;
            result |= pow2;
        }
    }
    return result;
}

u64 bit_by_bit_Q1648_naive(u32 N) { // N is actual u32, return type is Q16.48
    u128 remainder = (u128)N << 96;
    u64 result = 0;
    
    for(int i = 63; i >= 0 && remainder > 0; --i) {
        u64 pow2 = (u64)1 << i;
        u128 temp = (u128)((result << 1) + pow2) << i;
        if (temp <= remainder) {
            remainder -= temp;
            result |= pow2;
        }
    }
    return result;
}


void printfQ1616(u32 x) {
    printf(" sqrtq N: %f \n", (double)x/TWO_16);
}

void printfQ1648(u64 x) {
    printf(" sqrtq N: %f \n", (double)x/TWO_48);
}

void printfQ1648_precise(uint64_t x) {
    uint64_t integer_part = x >> 48;
    uint64_t frac_raw = x & 0xFFFFFFFFFFFFULL;
   	int num_dps = 18;

   	// get decimal fraction by multiplying by 10^ num dps then adjusting
   	// use 128 to avoid overflow
    u64 frac_dec = ((u128)frac_raw * (u128)1000000000000000000 + ((u64)1 << 47)) >> 48; // rounded
    
    printf(" sqrtq N: %llu.%018llu\n", integer_part, frac_dec);
}

float new_sqrt_v2(float x) {
    float xhalf = 0.5f * x;
    union {
        float x;
        int i;
    } u;
    u.x = x;
    u.i = 0x5f375a86 - (u.i >> 1);
    /* The next line can be repeated any number of times to increase accuracy */
    u.x = u.x * (1.5f - xhalf * u.x * u.x);
    u.x = u.x * (1.5f - xhalf * u.x * u.x);
    return u.x*x;
}

int main(){
	volatile u32 a;

	for(u32 i = 10;i<(u32)10000000;i++){
		//a = sqrtf((float)i);
		//a = bit_by_bit_Q1648(i);
		a = bit_by_bit_Q1616_best(i);
		//b = isqrt(i);
		//printfQ1648_precise(a);
		//printf("N: %lu",i);
		//printfQ1616(a);
		//	printf("N: %u sqrt N: %u actual sqrt N: %u \n",i,a,isqrt(i));
		//}
		//printf("%lf \n",new_sqrt_v2((float)i)-sqrt((float)i));
	}
}