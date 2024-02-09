#include "dbg\debug.h"
#include "list\list.h"
#include "simple-pebs.h"
#include "LBR\LBR.h"
#include "checks\checks.h"


#define _ECX_MAX_  0x10000
#define _ECX_MIN_  0x10
                                                                                                                                                                                                                                                                                                                                                                                                                                                  
extern PTDS_BASE DS_BASE;


BOOLEAN  bRepMov=FALSE ;
BOOLEAN  bTraceDLA=FALSE ;
UINT64 ddLastDataLA=0;
BOOLEAN bIPinBin=FALSE;
PLIST_ENTRY tempList=NULL;

UINT64  ddBlackList=0;

void MyCode()
{
	UINT64 ddEventingIP=0;
	UINT64 ddNextIP=0;
	UINT64 ddDataLA=0;
	UINT64 ecx=0;
	UINT64 esi=0;
	UINT64 edi=0;
	UINT64 esp=0;
	UINT64 currentEsp=0;
	UINT64 status;
	UINT64 flags;
	UINT64 dse; //Data Source Encoding
	UINT64 lat; //Latency value (core cycles)

	UINT64  ddStores=0;
	unsigned char * pByte=NULL; 
	int Layer=0;
	BOOLEAN bStoreOverflow=FALSE;
	struct pebs_v1 *pebs, *end;
	UINT64  pebs_index=0;
	PDLA 	tempDataLA=NULL;
	PLIST_ENTRY	removed_DLAofMyCode=NULL;
	PLoopList Find_content=NULL;

	PLoopList Find_Loop=NULL;
	PLIST_ENTRY nextLoop=NULL;
	PLIST_ENTRY nextDLA=NULL;
	PDATALA	DLA_Content=NULL;
	char Code[3]={0};
	 	KIRQL OldIrql;
		KIRQL OldIrql2;

	end = (struct pebs_v1 *)DS_BASE->pebs_index;
	if(DS_BASE->pebs_index>DS_BASE->pebs_base&&bPrint_pebs_index)
	{
		DbgPrint("pebs_index=%d",(DS_BASE->pebs_index-DS_BASE->pebs_base)/pebs_record_size);
		bPrint_pebs_index=FALSE;
	}

	 pebs = (struct pebs_v1 *)DS_BASE->pebs_base;
	 while(pebs< end)
	{
			 pebs = (struct pebs_v1 *)((char *)pebs + pebs_record_size);
	 }
	 pebs = (struct pebs_v1 *)((char *)pebs- pebs_record_size);

	

		ddEventingIP = ((struct pebs_v2 *)pebs)->eventing_ip&0x00000000FFFFFFFF;
		ddDataLA=((struct pebs_v1 *)pebs)->dataLA&0x00000000FFFFFFFF;
		ddNextIP=((struct pebs_v1 *)pebs)->ip&0x00000000FFFFFFFF;                                                                                                                                                                                                                                                                                                                   
		ecx=((struct pebs_v1 *)pebs)->cx;
		edi=((struct pebs_v1 *)pebs)->di;
		esi=((struct pebs_v1 *)pebs)->si;

		esp=((struct pebs_v1 *)pebs)->sp;
		status=((struct pebs_v1 *)pebs)->status;
		flags=((struct pebs_v1 *)pebs)->flags;
		dse=((struct pebs_v1 *)pebs)->dse;
		lat=((struct pebs_v1 *)pebs)->lat;

		pByte=(unsigned char *)ddEventingIP;
		bRepMov=FALSE;
		bTraceDLA=FALSE ;

		MyDbgPrint("[MyCode]  pebs_index%d: ddEventingIP=%08X,ddNextIP=%08X, ddDataLA=%08X ",pebs_index, ddEventingIP,ddNextIP,ddDataLA);
		pebs_index++;
		if( ddEventingIP>=_MAX_BIN_||ddNextIP>=_MAX_BIN_||ddDataLA>=_MAX_BIN_)
		{
			return;
		}

		if(ddEventingIP==ddNextIP)
		{
			DbgPrint("[MyCode]  pebs_index%d: ddEventingIP=%08X,ddNextIP=%08X, ddDataLA=%08X ",pebs_index, ddEventingIP,ddNextIP,ddDataLA);
		}
			get_LBR(DLAListHeader,ddEventingIP,ddNextIP, esp);

	return;
}