#ifndef _MICRON_MATH_H_
#define _MICRON_MATH_H_

#ifdef __cplusplus
	extern "C" {
#endif

//note, these use GCC extensions (typeof)
#define MIN(a, b) ({         \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a < _b ? _a : _b;       \
})
#define MAX(a, b) ({         \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b;       \
})

#ifndef abs
	#define abs(n) ({            \
		__typeof__ (n) _n = (n); \
		(_n < 0) ? -_n : _n;     \
	})
#endif

#define ROUND_TO_INT(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))


/** Reverse byte order of 32-bit integer.
 */
INLINE uint32_t byteswap32(uint32_t value) {
	uint32_t result;
	__asm__ volatile ("rev %0, %1" : "=r"(result) : "r"(value) );
	return result;
}

/** Reverse byte order of 16-bit integer.
 */
INLINE uint16_t byteswap16(uint16_t value) {
	uint32_t result;
	__asm__ volatile ("rev16 %0, %1" : "=r"(result) : "r"(value) );
	return result;
}

/** Reverse byte order of two 16-bit integers.
 */
INLINE uint32_t byteswap16_2(uint32_t value) {
	uint32_t result;
	__asm__ volatile ("rev16 %0, %1" : "=r"(result) : "r"(value) );
	return result;
}

/** Reverse byte order of signed short, and sign-extend.
 */
INLINE int32_t byteswapSigned(int32_t value) {
	int32_t result;
	__asm__ volatile ("revsh %0, %1" : "=r"(result) : "r"(value) );
	return result;
}

/** Rotate right unsigned 32-bit value.
 */
INLINE uint32_t ROR(uint32_t value, uint32_t count) {
	__asm__ volatile ("ror %0, %0, %1" : "+r"(value) : "r"(count) );
	return value;
}

#if 1 //XXX if CPU is Cortex M3 or greater

	/** Reverse bit order of 32-bit integer.
	 */
	INLINE uint32_t ReverseBits(uint32_t value) {
		uint32_t result;
		__asm__ volatile ("rbit %0, %1" : "=r"(result) : "r"(value) );
		return result;
	}

	/** Exclusive 8-bit load.
	 */
	INLINE uint8_t LDREXB(volatile uint8_t *addr) {
		uint8_t result;
		__asm__ volatile ("ldrexb %0, [%1]" : "=r"(result) : "r"(addr) );
		return result;
	}

	/** Exclusive 16-bit load.
	 */
	INLINE uint16_t LDREXH(volatile uint16_t *addr) {
		uint16_t result;
		__asm__ volatile ("ldrexh %0, [%1]" : "=r"(result) : "r"(addr) );
		return result;
	}

	/** Exclusive 32-bit load.
	 */
	INLINE uint32_t LDREXW(volatile uint32_t *addr) {
		uint32_t result;
		__asm__ volatile ("ldrexw %0, [%1]" : "=r"(result) : "r"(addr) );
		return result;
	}

	/** Exclusive 8-bit store.
	 *  Returns 0 on success, 1 on failure.
	 */
	INLINE int STREXB(volatile uint8_t *addr, uint8_t value) {
		int result;
		__asm__ volatile ("strexb %0, %2, [%1]" :
			"=r"(result) : "r"(addr), "r"(value) );
		return result;
	}

	/** Exclusive 16-bit store.
	 *  Returns 0 on success, 1 on failure.
	 */
	INLINE int STREXH(volatile uint16_t *addr, uint16_t value) {
		int result;
		__asm__ volatile ("strexh %0, %2, [%1]" :
			"=r"(result) : "r"(addr), "r"(value) );
		return result;
	}

	/** Exclusive 32-bit store.
	 *  Returns 0 on success, 1 on failure.
	 */
	INLINE int STREXW(volatile uint32_t *addr, uint32_t value) {
		int result;
		__asm__ volatile ("strexw %0, %2, [%1]" :
			"=r"(result) : "r"(addr), "r"(value) );
		return result;
	}

	/** Remove exclusive lock created by LDREX.
	 */
	INLINE void CLREX() {
		__asm__ volatile ("clrex");
	}

	/** Saturate signed value
	 */
	INLINE int32_t saturateSigned(int32_t val, int pos) {
		int32_t result;
		__asm__ volatile ("ssat %0, %1, %2" :
			"=r"(result) : "I"(pos), "r"(val) );
		return result;
	}

	/** Saturate unsigned value
	 */
	INLINE uint32_t saturateUnsigned(uint32_t val, int pos) {
		uint32_t result;
		__asm__ volatile ("usat %0, %1, %2" :
			"=r"(result) : "I"(pos), "r"(val) );
		return result;
	}

	/** Count leading zeros
	 */
	INLINE uint8_t countLeadingZeros(uint32_t val) {
		uint8_t result;
		__asm__ volatile ("clz %0, %1" : "=r"(result) : "r"(val) );
		return result;
	}

#endif //if Cortex-M3 or greater


#ifdef __cplusplus
	} //extern "C"
#endif

#endif //_MICRON_MATH_H_
