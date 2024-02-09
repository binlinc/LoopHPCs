#pragma once

#include "config.h"

  INT32  MagicCheck(DWORD64 Address , DWORD64 ddDataLAMax);
	  BOOLEAN  HeapCheck(DWORD32 lowAddress );

#define RVA_Code 0x1000
#define RVA_Rdata 0x6000
#define RVA_Data 0x7000
#define RVA_End 0xA000


#define MAGIC_Code 0xCC
#define MAGIC_Rdata 0xBB
#define MAGIC_Data 0xAA



