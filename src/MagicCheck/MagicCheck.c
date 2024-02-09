#include "config.h"

#define _HEAP_MIN_   0x00400000 // 0x02000000  
#define _HEAP_MAX_ 0x00410000 //0x03000000  

BOOLEAN  HeapCheck(DWORD32 lowAddress )
{
	 // [TODO]
	return TRUE;

	if( lowAddress>=_HEAP_MIN_ && lowAddress<_HEAP_MAX_ )
	{
		return TRUE;
	}
	return FALSE;
}

	  INT32  MagicCheck(DWORD64 Address , DWORD64 ddDataLAMax)
{
	PDWORD64 pStartAddress=0;
	DWORD32 lowAddress=0;
	PCHAR pStartAddressByte=0;

	lowAddress=Address&0xFFFFFFFF;

	if( HeapCheck (lowAddress)==FALSE )
	{
		return 0;
	}

	pStartAddress=(PDWORD64) (Address&0xFFFFFFFFFFFF0000);

	DbgPrint(" MagicCheck [%08X-->%08X]",pStartAddress, ddDataLAMax);
	 pStartAddressByte=(PCHAR)pStartAddress;
	 for( ; (DWORD64)pStartAddressByte<ddDataLAMax; pStartAddressByte++)
	 {
		 if((*pStartAddressByte)!=0)
		 {
			 DbgPrint("[%08X]=%X",pStartAddressByte,*(pStartAddressByte));
		 }
	 }

	return 1;
}