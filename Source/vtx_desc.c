#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/datatypes.h>

/* we don't have our class library base yet, therefore we use the resources from the given DTHookContext */
#undef SysBase
#undef DOSBase
#undef UtilityBase
#define SysBase     (dthc -> dthc_SysBase)
#define UtilityBase (dthc -> dthc_UtilityBase)
#define DOSBase     (dthc -> dthc_DOSBase)

#undef IExec
#undef IDOS
#undef IUtility
#define IExec     (dthc -> dthc_IExec)
#define IUtility (dthc -> dthc_IUtility)
#define IDOS     (dthc -> dthc_IDOS)

BOOL _start( struct DTHookContext *dthc )  // DTHook
{
	struct FileInfoBlock *fib = dthc->dthc_FIB;
	long size=0;
	char match[14];

	if(dthc->dthc_Buffer[0] == 'J' && dthc->dthc_Buffer[1] == 'W' &&
			dthc->dthc_Buffer[2] == 'C' && dthc->dthc_Buffer[4] == 0x00 &&
			dthc->dthc_Buffer[5] == 0x00)
		{
			return(TRUE);
		}

	if(dthc->dthc_Buffer[0] == 0xFE && dthc->dthc_Buffer[1] == 0x01 &&
			dthc->dthc_Buffer[2] == 0x09 && dthc->dthc_Buffer[3] == 0x00 &&
			dthc->dthc_Buffer[4] == 0x00 && dthc->dthc_Buffer[5] == 0x00)
		{
			return(TRUE);
		}

	if(fib->fib_Size == 960)
	{
		IDOS->ParsePatternNoCase("#?.BIN",&match,14);
		if(IDOS->MatchPatternNoCase(&match,fib->fib_FileName))
		{
			return(TRUE); // FRAME
		}
	}

	size = fib->fib_Size / 960;
	if(size*960 == fib->fib_Size)
	{
		IDOS->ParsePatternNoCase("#?.CAR",&match,14);
		if(IDOS->MatchPatternNoCase(&match,fib->fib_FileName))
		{
			return(TRUE); // carousel
		}
	}

    return(FALSE);
}
