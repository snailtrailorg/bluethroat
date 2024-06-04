#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/utilities/sme_float.h"

int main(void) {
	float32_t TSF((int32_t)516096);
	float32_t PSF((int32_t)1040384);
	float32_t C0((int32_t)222);
	float32_t C1((int32_t)-295);
	float32_t C00((int32_t)82580);
	float32_t C10((int32_t)-55343);
	float32_t C01((int32_t)-3496);
	float32_t C11((int32_t)1545);
	float32_t C20((int32_t)-11412);
	float32_t C21((int32_t)216);
	float32_t C30((int32_t)-1756);

	C0 /= 2;
	printf("C0(%e) = s(%d), m(0x%08x), e(%d)\n", (float)C0, (int)(C0.s), (unsigned int)(C0.m), (int)(C0.e));

	C10 /= PSF;
	printf("C10(%e) = s(%d), m(0x%08x), e(%d)\n", (float)C10, (int)(C10.s), (unsigned int)(C10.m), (int)(C10.e));

	C30 /= PSF;
	C30 /= PSF;
	C30 /= PSF;
	printf("C30(%e) = s(%d), m(0x%08x), e(%d)\n", (float)C30, (int)(C30.s), (unsigned int)(C30.m), (int)(C30.e));

	return 0;
}
