 #include "config.h"
#include "strsafe.h"

BOOLEAN WriteFileA(char szBuffer[] );
BOOLEAN  WriteFileW(WCHAR wBuf[]);

#define MAX_PATH 256
char szBuf[MAX_PATH];
WCHAR wBuf[MAX_PATH];


extern char szProcessName [MAX_PATH];
extern char szLogFileName [MAX_PATH];
extern int iMLPX;

UNICODE_STRING Unicode_ProcessName;


#define LOG_BUILD 1
#define LOG2FILEA(fmt, ...) \
	do { if (LOG_BUILD){ StringCbPrintfA(szBuf,_MAX_LEN_,fmt, ##__VA_ARGS__ ); }  WriteFileA(szBuf);} while (0)


#define LOG2FILEW(fmt, ...) \
	do { if (LOG_BUILD){ StringCbPrintfW(wBuf,_MAX_LEN_,fmt, ##__VA_ARGS__ ); }  WriteFileW(wBuf);} while (0)