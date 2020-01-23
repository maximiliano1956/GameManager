#ifndef POLYKEYS_H
#define POLYKEYS_H


#if definde(_MSC_VER)
#  define U64_POLY(u) (u##ui64)
#else
#  define U64_POLY(u) (u##ULL)
#endif

extern U64 Random64Poly[];

#endif
