#pragma once
#include <cstdint>
typedef unsigned long DWORD;
typedef unsigned long long DWORD_PTR;
#define NORMAL_PRIORITY_CLASS 0x00000020
typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
typedef struct _DEVMODEW {
    short dmSize;
} DEVMODEW;
