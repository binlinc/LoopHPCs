

#include "debug.h"
#include "checks\checks.h"

/* DbgPrint wrapper
 * It is a way of automatically printing driver name every call
 * Useful for debug filtering -- string-based
 */
void debug(char msg[])
{
#ifdef DEBUG
	DbgPrint("%s %s",DRIVER_NAME,msg);
#else
	UNREFERENCED_PARAMETER(msg);
	return;
#endif
}



/* Printing error message associated to an error code */
void emit_error(int code){
	DbgPrint("%s ERROR: %s",DRIVER_NAME,enum_string[code]);
}





void GetCurrentTimeString()
{
	static CHAR  szTime[128];
	LARGE_INTEGER SystemTime;
	LARGE_INTEGER LocalTime;
	TIME_FIELDS  timeFiled;

	KeQuerySystemTime(&SystemTime);
	ExSystemTimeToLocalTime(&SystemTime, &LocalTime);
	RtlTimeToTimeFields(&LocalTime, &timeFiled);
	DbgPrint("%02d:%02d:%02d"
		, timeFiled.Hour
		, timeFiled.Minute
		, timeFiled.Second
		);

}