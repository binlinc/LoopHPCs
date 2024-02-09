
#include "MagicCheck/MagicCheck.h"
#include "list.h"
#include "dbg\debug.h"
#include "simple-pebs.h"
#include "WriteFile\WriteFile.h"


enum SortType
{
	Type_Counter, 
	Type_Time,
	Type_Address
};

#define _SortType_ Type_Address


KSPIN_LOCK list_lock;
KIRQL irq2;
int LenofLoopList=0;
extern PCHAR szBuffer_BASE;

UINT64  getBaseAddress(UINT64 address)
{
	if(address>=_Original_Code_&&address<_Original_Rdata_)
	{
		return _Original_Code_;
	}
	else
	{
		return address;
	}
}


int ListLength(PLIST_ENTRY List_Header )
{
	PLIST_ENTRY tempEntry=NULL;
	int i=0;
	if(List_Header==NULL)
	{
		return 0;
	}
	tempEntry=List_Header;
	i=0;
	do{
		tempEntry=tempEntry->Flink;
		i++;
	}while(tempEntry!=List_Header);
	i--;
	return i;
}

BOOLEAN create_list(PLIST_ENTRY *List_Header )
{
	if(*List_Header!=NULL)
	{
		return TRUE;
	}
	KeInitializeSpinLock(&list_lock);
	*List_Header=(PLIST_ENTRY)ExAllocatePool(NonPagedPool,sizeof(LIST_ENTRY));
	if(*List_Header==NULL)
	{
		DbgPrint("Allocation List_Header Error !");
		DbgPrint("*List_Header=%08X", *List_Header);
		return FALSE;
	}
	else
	{
		//DbgPrint("Allocation List_Header suscess !");
		InitializeListHead(*List_Header);
	}
	return TRUE;
}
//Delete the inner loop
void CleanLoop()
{
	PLIST_ENTRY Current_Entry=NULL;
	PLIST_ENTRY Next_Entry=NULL;
	PLIST_ENTRY Temp_Entry=NULL;
	PLoopList Current_Content=NULL;
	PLoopList Next_Content=NULL;

	UINT64 MaxCounter=0;
	UINT64 MaxFrom=0;
	UINT64 PMC=0;

	if(LoopList_Header==NULL || IsListEmpty(LoopList_Header))
	{
		return;
	}
		
	ExAcquireSpinLock(&list_lock,&irq2);
	//Search the Max_content in term of LoopInfo.Counter.
	for(Current_Entry = LoopList_Header->Flink ; Current_Entry != LoopList_Header; Current_Entry = Current_Entry->Flink  )
	{
		Current_Content = CONTAINING_RECORD(Current_Entry, LoopList, loopList);
		for(Next_Entry = Current_Entry->Flink ; Next_Entry != Current_Entry;  )
		{
			Next_Content = CONTAINING_RECORD(Next_Entry, LoopList, loopList);
			if(	 Next_Content->LoopInfo.FROM<=Current_Content->LoopInfo.FROM
				&& Next_Content->LoopInfo.TO>=Current_Content->LoopInfo.TO)
			{
				Temp_Entry=Next_Entry;
				Next_Entry = Next_Entry->Flink;
				RemoveEntryList(Temp_Entry);
			}//delete entry		
		}
	}//end of 	Search
	KeReleaseSpinLock(&list_lock,irq2);
}
void destroy_list( )
{
	DbgPrint("[destroy_list] MLPX is %d\n",iMLPX);

	//CleanLoop();

	//Print and Remove the loop list
	if(LoopList_Header!=NULL)
	{
		while(!IsListEmpty(LoopList_Header))
		{
			Remove_LoopList(LoopList_Header);
		}
		ExFreePool(LoopList_Header);
	}


	return;
}


BOOLEAN Remove_LoopList(PLIST_ENTRY LoopList_Header )
{
	PLIST_ENTRY Next_Entry=NULL;
	PLIST_ENTRY Max_Entry=NULL;
	PLoopList next_content=NULL;
	PLoopList Max_content=NULL;
	int i,j=0;
	UINT64 MaxCounter=0;
	UINT64 MaxFrom=0;
	UINT64 PMC=0;

	/*
	ExAcquireSpinLock(&list_lock,&irq2);
	//Search the Max_content in term of LoopInfo.Counter.
	for(Next_Entry = LoopList_Header->Flink ; Next_Entry != LoopList_Header; Next_Entry = Next_Entry->Flink  )
	{
		next_content = CONTAINING_RECORD(Next_Entry, LoopList, loopList);

		if (_SortType_ ==Type_Counter)
		{
			if(	 next_content->LoopInfo.Counter>MaxCounter)
			{
				MaxCounter=next_content->LoopInfo.Counter;
				Max_Entry=  Next_Entry;
			}
		}
		else if (_SortType_ ==Type_Address)
		{
			if(	 ((next_content->LoopInfo.FROM)&0x000000000000FFFF)>MaxFrom)
			{
				MaxFrom= (next_content->LoopInfo.FROM)&0x000000000000FFFF;
				Max_Entry=  Next_Entry;
			}
		}
		else
		{
			Max_Entry= LoopList_Header->Flink;
		}
	}//end of 	Search
	KeReleaseSpinLock(&list_lock,irq2);
	*/

	//Print the Entry in loop list
	Max_Entry= LoopList_Header->Flink;
	Max_content=CONTAINING_RECORD(Max_Entry,LoopList,loopList);
	//If Max_content->LoopInfo.Counter=1, we can't calculate the delta of PEBs
	//MLPX
	if(Max_content->LoopInfo.Counter>1 &&
		(! (Max_content->LoopInfo.FROM>=_Original_Code_ && Max_content->LoopInfo.FROM<_Original_Rdata_) )
		)
	{	
		StringCbPrintfA(szBuffer_BASE, szBuffer_SIZE,"0x%08X,0x%08X,0x%08X,%d",((Max_content->LoopInfo.FROM)&0x000000000000FFFF),Max_content->LoopInfo.FROM, Max_content->LoopInfo.TO, Max_content->LoopInfo.Counter);
		WriteFileA(szBuffer_BASE);
		
		for(i=0; i<iMLPX; i++)
		{
			for(j=OutSet; j<MAX_PMCx; j++)
			{
				PMC=iMLPX*(Max_content->LoopInfo.Counter,Max_content->LoopInfo.PMC_End[i][j]-Max_content->LoopInfo.PMC_Start[i][j]);
				StringCbPrintfA(szBuffer_BASE, szBuffer_SIZE,",%lld", PMC);
				WriteFileA(szBuffer_BASE);
			}
		}
		WriteFileA("\r\n");
	}

	//Remove the Entry in loop list
	ExFreePool(Max_content);
	RemoveEntryList(Max_Entry);
	LenofLoopList--;
	return TRUE;
}



DWORD64  CheckThrehold=0x200; 


BOOLEAN Find_Insert_LoopList(UINT64  FROM,UINT64 TO,UINT64 Counter, UINT64 ESP,UINT64 INST,  UINT64 CYCLE,UINT64 CurrentPMC[],PLIST_ENTRY DLAHeader)
{	
	PLoopList Find_Loop=NULL;
	PLIST_ENTRY next=NULL;
	PLIST_ENTRY nextDLA=NULL;
	PDLA DLA_Content=NULL;
	PLoopList tempData=NULL;
	PDATALA tempDataLA=NULL;
	UINT64 Layer=0;
	BOOLEAN bRepMov=FALSE;
	BOOLEAN bFindLoop=FALSE;
	UINT64 currentEsp=0;
	UINT64 ddDataLA=0;
	int i,j=0;


	if(LoopList_Header==NULL)
	{
		DbgPrint("[Find_Insert_LoopList] LoopList_Header is NULL!");
		return FALSE;
	}

	// Loop search
	for(next = LoopList_Header->Flink  ; next != LoopList_Header; next = next->Flink  )
	{
		Find_Loop = CONTAINING_RECORD(next, LoopList, loopList);

		//negative branch, loop?
		if(FROM>TO&&FROM-TO<0x1000)
		{
			//  Match Exising Loop
			if( Find_Loop->LoopInfo.TO==TO&&Find_Loop->LoopInfo.FROM==FROM)
			{
				bFindLoop=TRUE;
				MyDbgPrint("Matched Loop: %08X-->%08X, Counter=%d",FROM, TO, Find_Loop->LoopInfo.Counter+1 );
				// Enter the loop again
				if(Find_Loop->LoopInfo.bActive==FALSE)
				{
					MyDbgPrint("Enter Loop again: %08X-->%08X", Find_Loop->LoopInfo.FROM, Find_Loop->LoopInfo.TO);
					Find_Loop->LoopInfo.INST_Start+=INST-Find_Loop->LoopInfo.INST_End;
					Find_Loop->LoopInfo.CYCLE_Start+=CYCLE-Find_Loop->LoopInfo.CYCLE_End;
					for(i=OutSet; i<MAX_PMCx;i++)
					{
						Find_Loop->LoopInfo.PMC_Start[Multiplex_Index][i]+=CurrentPMC[i];
					}

					Find_Loop->LoopInfo.bActive=TRUE;
				}

		//		Find_Loop->LoopInfo.INST_End=INST;
		//		Find_Loop->LoopInfo.CYCLE_End=CYCLE;
				for(i=OutSet; i<MAX_PMCx;i++)
				{
					Find_Loop->LoopInfo.PMC_End[Multiplex_Index][i]+=CurrentPMC[i];
				}

			//	Find_Loop->LoopInfo.ESP=ESP;
			//	Layer=Find_Loop->LoopInfo.Layer;	

				Find_Loop->LoopInfo.Counter++;
				Multiplex_Index= (Find_Loop->LoopInfo.Counter)%iMLPX;

			} 	//  End of Match Exising Loop
		/*	else
			{
				DbgPrint("Not Matched Loop : %08X-->%08X", Find_Loop->LoopInfo.FROM, Find_Loop->LoopInfo.TO);
			
			}
			*/
		}//End of  negative branch 

		//determine the "bActive"
		//Inner the  Find_Loop
		if( TO>=Find_Loop->LoopInfo.TO &&TO<=Find_Loop->LoopInfo.FROM)
		{
			Find_Loop->LoopInfo.bActive=TRUE;
			MyDbgPrint(" IP: %08X-->%08X is within  the loop %08X-->%08X", FROM, TO, Find_Loop->LoopInfo.FROM, Find_Loop->LoopInfo.TO);
		}
		else
		{
			MyDbgPrint("IP: %08X-->%08X  exit  the loop %08X-->%08X", FROM, TO, Find_Loop->LoopInfo.FROM, Find_Loop->LoopInfo.TO);
			Find_Loop->LoopInfo.bActive=FALSE;
		}//End of determining the "bActive"

	}//End of  Loop search

	//Since current Loop does not match any existing loop,  we create a new Loop.
	if( (FROM>TO&&FROM-TO<0x1000)
		&& 	bFindLoop==FALSE)
	{	
		//TO-DbgPrint
		MyDbgPrint("New Loop: %08X-->%08X", FROM, TO);
		// Record 0 PEBs value for the first item of loop
		// Insert_LoopList(  FROM, TO, Counter, 0,  0, 0, ddDataLA,ddOriginalCode);
		tempData=(PLoopList)ExAllocatePool(NonPagedPool,sizeof(LoopList));
		if(tempData==NULL)
		{
			DbgPrint("[Insert_LoopList]	 Allocate tempData	 Error");
			return FALSE;
		}
		tempData->LoopInfo.FROM=FROM;
		tempData->LoopInfo.TO=TO;
		tempData->LoopInfo.Counter=	1;
		tempData->LoopInfo.bActive=TRUE;

		for(i=0; i<iMLPX; i++)
		{
			for(j=OutSet; j<MAX_PMCx;j++)
			{
				tempData->LoopInfo.PMC_Start[i][j]=0;
				tempData->LoopInfo.PMC_End[i][j]=0;
			}
		}

		InsertTailList(LoopList_Header,&(tempData->loopList));
		LenofLoopList++;
		lastLoop=tempData;
	}// end of new create loop

	return TRUE;
}
