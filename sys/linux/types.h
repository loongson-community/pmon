#ifndef __LINUX_TYPES_H_

#define __LINUX_TYPES_H_

typedef unsigned long long u64;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;

typedef signed char                s8;
typedef signed short               s16;
typedef signed long                s32;

/*
#if (_MIPS_SZPTR == 64)
    typedef __signed__ long s64;
    typedef unsigned long u64;
#else
    #if defined(__GNUC__) && !defined(__STRICT_ANSI__)
        typedef __signed__ long long s64;
        typedef unsigned long long u64;
    #endif
#endif
*/

typedef signed int                 sint;

typedef signed long slong;

#endif /* __LINUX_TYPES_H_ */
