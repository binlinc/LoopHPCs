#include "config.h"
#include "simple-pebs.h"


typedef struct st_DLAofMyCode
{
	UINT64  DataLA;
	BOOLEAN  bRepMov;
	UINT64  EventingIP;
	UINT64 NextIP;
	LIST_ENTRY DataLAList;
}DLA,*PDLA;


typedef struct st_DataLA
{
	UINT64  ddDataLAMin;
	UINT64 ddDataLAMax;
	LIST_ENTRY DataLAList;

}DATALA,*PDATALA;


typedef struct st_LayerList
{
	UINT64 ddDataLANode;
	UINT64 ddOriginalPEHeader;
	UINT64	ddOriginalCode;
	UINT64	ddOriginalRdata;
	UINT64	ddOriginalData;
	UINT64 	OriginalDLA;
	UINT64	INST;
	UINT64	CYCLE;
	UINT64 	PMC[Multiplexing][MAX_PMCx];


	LIST_ENTRY  layerList ;

}LayerList,*PLayerList;

typedef struct st_LOOPInfo
{
	UINT64  Layer;
	BOOLEAN bActive; // Is the EIP within the loop?
	BOOLEAN bOriginaCode; // Does the loop generate original code?
	UINT64	FROM;
	UINT64	TO;
	UINT32  Counter;
	//Start Value
	UINT64	INST_Start;
	UINT64	CYCLE_Start;
	UINT64 	PMC_Start[Multiplexing][MAX_PMCx];

	//End Value
	UINT64	INST_End;
	UINT64	CYCLE_End;
	UINT64 	PMC_End[Multiplexing][MAX_PMCx];

	UINT64  ESP;

	UINT64  ddOriginalPEHeader;
	UINT64	ddOriginalCode;
	UINT64	ddOriginalRdata;
	UINT64	ddOriginalData;
}LOOPInfo,*PLOOPInfo;


typedef struct st_list_data
{
	LOOPInfo LoopInfo;
	LIST_ENTRY loopList;
}LoopList,*PLoopList;


/* List operations */
BOOLEAN Remove_LoopList(PLIST_ENTRY LoopList_Header );
BOOLEAN Find_Insert_LoopList(UINT64  FROM,UINT64 TO,UINT64 Counter, UINT64 ESP, UINT64 INST, UINT64 CYCLE,UINT64 CurrentPMC[],PLIST_ENTRY DLAHeader);
BOOLEAN create_list(PLIST_ENTRY *List_Header );
void destroy_list();
BOOLEAN DLA2Layer(UINT64 Layer_Index, PLIST_ENTRY DLAHeader );
int FindLayer(UINT64  FROM,UINT64 TO);
int ListLength(PLIST_ENTRY List_Header );


PLIST_ENTRY LoopList_Header;
// For "exit-then-enter"
PLoopList lastLoop;
PLIST_ENTRY DLAListHeader;

#define  _MAX_BIN_   0x04000000  
//#define  _MAX_BIN_   0x10000000 
#define _SUCCESSIVE_VALUE_  0x200
#define  _CLib_MIN_   0x00400000
#define  _CLib_MAX_ 0x00401000
#define  _StackTrace_MAX_ 0x10
#define  _Original_PEHeader_ 0x00400000  
#define  _Original_Code_		0x00401000  
#define  _Original_Rdata_	0x00736000   
#define  _Original_Data_		0x00737000
#define  _Original_PETail_   0x00737200  
#define _Original_DLA_Min	0x007370E2 
#define _Original_DLA_Max	0x007371AA 


#define MIN(X,Y) ((X)>(Y)?(Y):(X))
#define MAX(X,Y) ((X)<(Y)?(Y):(X))

#define szBuffer_SIZE 1000