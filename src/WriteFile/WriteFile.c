#include "config.h"
#include "dbg/debug.h"
#include "WriteFile.h"
#include "..\simple-pebs.h"

#define MAX_PROC_NAME_LEN 256


UNICODE_STRING LogFileNameA=RTL_CONSTANT_STRING(L"\\??\\F:\\HPCs.csv");
UNICODE_STRING LogFileNameW=RTL_CONSTANT_STRING(L"\\??\\F:\\DriverW.log");
UNICODE_STRING Unicode_FileNameHeader=RTL_CONSTANT_STRING(L"\\??\\");
UNICODE_STRING Unicode_DirectoryHeader=RTL_CONSTANT_STRING(L"\\");
UNICODE_STRING Unicode_FileName = { 0 };
UNICODE_STRING Unicode_DirectoryName = { 0 };
WCHAR wcstr[MAX_PROC_NAME_LEN] = { 0 };

WCHAR wcTemp[100] = { 0 };
char szColName [1000]={0};
char szTempName [256]={0};

char szEventName[4][100]={"BRANCHS","ICACHE_Misses","STORES","LOADS"};


BOOLEAN  IniteFileName()
{
	NTSTATUS    ntStatus;
	ANSI_STRING AnsiString;
	UNICODE_STRING UnicodeString;

	RtlInitEmptyUnicodeString(&Unicode_FileName, wcstr, sizeof(wcstr));
	RtlCopyUnicodeString(&Unicode_FileName,&Unicode_FileNameHeader);

	DbgPrint("Unicode_ProcessName=%wZ",&Unicode_ProcessName);
	ntStatus=RtlAppendUnicodeStringToString (&Unicode_FileName,&Unicode_ProcessName);
	if(Unicode_FileName.Length>4)
	{
		Unicode_FileName.Length=Unicode_FileName.Length-8;
	}
	DbgPrint("Unicode_FileName=%wZ",&Unicode_FileName);
	RtlInitEmptyUnicodeString(&Unicode_DirectoryName, wcstr, sizeof(wcstr));
	RtlCopyUnicodeString(&Unicode_DirectoryName,&Unicode_FileName);
	DbgPrint("Unicode_DirectoryName=%wZ",&Unicode_DirectoryName);

	DbgPrint("szLogFileName=%s",szLogFileName);
	RtlInitAnsiString(&AnsiString,	szLogFileName);
	DbgPrint("AnsiString=%Z",&AnsiString);
	RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
	RtlAppendUnicodeStringToString (&Unicode_FileName,&Unicode_DirectoryHeader);
	RtlAppendUnicodeStringToString (&Unicode_FileName,&UnicodeString);
	DbgPrint("Unicode_FileName=%wZ",&Unicode_FileName);

	if(ntStatus==STATUS_SUCCESS)
	{
		return TRUE;
	}
	else
	{
		DbgPrint("RtlAppendUnicodeStringToString Fail !");
		return FALSE;
	}	
}


BOOLEAN bHasColname=TRUE;


NTSTATUS CreateDirectory(UNICODE_STRING *pDirName)//L"\\??\\c:\\doc\\"
{
	OBJECT_ATTRIBUTES		objAttrib = { 0 };
	UNICODE_STRING			uDirName = { 0 };
	IO_STATUS_BLOCK 		io_status = { 0 };
	HANDLE					hFile = NULL;
	NTSTATUS				status = 0;

//	RtlInitUnicodeString(&uDirName, szDirName);
	InitializeObjectAttributes(&objAttrib,
		pDirName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL);

	status = ZwCreateFile(&hFile,
		GENERIC_READ | GENERIC_WRITE,
		&objAttrib,
		&io_status,
		NULL,
		FILE_ATTRIBUTE_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);
	if (NT_SUCCESS(status))
	{
		ZwClose(hFile);
	}

	return status;
}


BOOLEAN WriteFileA(char szBuffer[] )
{
	OBJECT_ATTRIBUTES object_attributes;
	IO_STATUS_BLOCK ioStatus;
	HANDLE  hFile;
	UNICODE_STRING  log;
	NTSTATUS status;
	PUCHAR	pBuffer;
	ULONG BufferSize=0;
	ULONG ColSize=0;
	int i,j=0;
	IO_STATUS_BLOCK file_status;
	FILE_STANDARD_INFORMATION fsi;
	FILE_POSITION_INFORMATION fpi;


	if (PASSIVE_LEVEL !=  KeGetCurrentIrql()) 
	{
		DbgPrint("[WriteFileA Error] Irql is not  PASSIVE_LEVEL !");
		return FALSE;
	}

	if(IniteFileName()==TRUE)
	{
		LogFileNameA=Unicode_FileName;
	}
	
	CreateDirectory(&Unicode_DirectoryName);
	InitializeObjectAttributes(&object_attributes, &LogFileNameA, OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

	status = ZwCreateFile(&hFile,
			GENERIC_READ|GENERIC_WRITE,
		&object_attributes,
		&ioStatus,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			FILE_OPEN_IF,
			FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT,
			NULL,
			0);
	if (!NT_SUCCESS(status)){
		DbgPrint("[WriteFileA Error] Create LogFile Failed !");
		return STATUS_SUCCESS;
	}
	else
	{
		MyDbgPrint("[WriteFileA Success] Create LogFile Success !");
	}


	ZwQueryInformationFile(hFile,
		&file_status,
		&fsi,
		sizeof(FILE_STANDARD_INFORMATION), 
		FileStandardInformation);

	fpi.CurrentByteOffset = fsi.EndOfFile;        

	MyDbgPrint("fsi.EndOfFile:%lld\n", fsi.EndOfFile);


	BufferSize=(ULONG)strnlen_s(szBuffer,256);
	if(  (ULONG)fsi.EndOfFile.LowPart==0 )
	{
		DbgPrint("[WriteFileA] MLPX is %d\n",iMLPX);
		StringCbCopyA(szColName,sizeof (szColName),"Address,From,To, Counter");
			for(i=0; i<iMLPX; i++)
			{
				for(j=OutSet; j<MAX_PMCx; j++)
				{
					StringCbPrintfA(szTempName, sizeof (szTempName), ",%s%d", szEventName[j], i );
					StringCbCatA(szColName ,sizeof (szColName),szTempName);
				}
			}		
		StringCbCatA(szColName ,sizeof (szColName),"\r\n");
		ColSize=(ULONG)strnlen_s(szColName,sizeof (szColName));
		BufferSize+=ColSize;
	}

	pBuffer = (PUCHAR)ExAllocatePool(PagedPool, BufferSize+1);
	if (pBuffer==NULL){
		DbgPrint("[Error]  Alllocate pBuffer Failed !");
		return FALSE;
	}
	memset(pBuffer, 0, BufferSize+1);
	if(ColSize!=0)
	{
		memcpy(pBuffer, szColName,ColSize);
	}
	memcpy(pBuffer+ColSize, szBuffer,BufferSize-ColSize);
	MyDbgPrint("pBuffer=%s\n", pBuffer);

	ZwSetInformationFile(hFile,
		&file_status,
		&fpi, 
		sizeof(FILE_POSITION_INFORMATION),
		FilePositionInformation);

	ZwWriteFile(hFile, NULL, NULL, NULL, &ioStatus, pBuffer, BufferSize, NULL, NULL);
	MyDbgPrint("WriteByte:%d\n", ioStatus.Information);

	ExFreePool(pBuffer);
	ZwClose(hFile);

	ColSize=0;
	return TRUE;
}