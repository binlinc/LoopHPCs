

#include "dbg\debug.h"
#include "checks\checks.h"
#include "simple-pebs.h"
#include "LBR\LBR.h"

/* perform all checks at once */
int perform_checks()
{
	int ret_code;
	UINT64 cap;
	debug("Performing Checks");


	/* check if CPU is Intel
	 * AMD processor uses another kind of branch monitor
	 */
	if((ret_code=CPU_check())!=NO_ERROR)
	{
		return CPU_IS_NOT_INTEL;
	}
	/* check if APIC was changed */
	if((ret_code=check_apic())!=NO_ERROR)
	{
		return APIC_WAS_REMAPPED;
	}
	/* Check if CPU is x64
	 * BTS supports x86; the current implm. doesn't
	 * TODO: change structs to support UINT32
	 */
	if((ret_code=check_is_64())!=NO_ERROR)
	{
		return CPU_IS_NOT_64;
	}
	/* Check if CPU has DS support
	 * This is required for writing on O.S pages
	 * If it doesn't, use LBR instead BTS
	 */
	if((ret_code=check_has_ds())!=NO_ERROR)
	{
		return CPU_HAS_NO_DS;
	}
	/* check BTS support itself */
	if((ret_code=check_has_bts())!=NO_ERROR)
	{
		debug("BTS not support");
		return CPU_HAS_NO_BTS;
	}
	debug("BTS support");

	/* Probably more checks are available
	 * TODO: Check for PERF_COUNT support
	 */
	cap=__readmsr(MSR_IA32_PERF_CAPABILITIES);
	pebs_record_size = 0;
	PEBs_V=(cap >> 8) & 0xf;
		switch (PEBs_V) {
		case 1:
			pebs_record_size = sizeof(struct pebs_v1);
				DbgPrint("pebs_v1  ");
					LBR_SIZE=8;
			break;
		case 2:
			pebs_record_size = sizeof(struct pebs_v2);
			DbgPrint("pebs_v2  ");
				LBR_SIZE=16;
			break;
		case 3:
			pebs_record_size = sizeof(struct pebs_v3);
			LBR_SIZE=32;
				DbgPrint("pebs_v3  ");
			break;
		default:
				DbgPrint("pebs_record_size=%d",pebs_record_size);
				return Unsupported_PEBS_Format;
		}

	return NO_ERROR;
}



