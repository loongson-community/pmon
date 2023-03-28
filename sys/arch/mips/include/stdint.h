#ifndef _STDINT_H
#define _STDINT_H

#ifndef __BIT_TYPES_DEFINED__
#if defined _LP64 || defined _I32LPx || defined __LP64__ || (defined __WORDSIZE && __WORDSIZE == 64) || (_MIPS_SZLONG == 64 && _MIPS_SZINT == 32)
typedef long int int64_t;
typedef unsigned long int uint64_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int int16_t;
typedef unsigned short int uint16_t;
#elif defined _LLP64 || defined __LLP64__ || _MIPS_SZLONG == 32
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
typedef long int int32_t;
typedef unsigned long int uint32_t;
typedef short int int16_t;
typedef unsigned short int uint16_t;
#else
#error "Type model not supported"
#endif
typedef signed char int8_t;
typedef unsigned char uint8_t;
#define __BIT_TYPES_DEFINED__
#endif

#endif
