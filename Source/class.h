/*
 * videotex.datatype
 */

#ifndef ICO_CLASS_H
#define ICO_CLASS_H

#include <exec/exec.h>
#include <dos/dos.h>
#include <utility/utility.h>
#include <datatypes/pictureclass.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/datatypes.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>

struct ClassBase {
	struct Library	libNode;
	uint16			Pad;
	Class			*DTClass;
	BPTR			SegList;

	struct ExecIFace		*IExec;
	struct DOSIFace			*IDOS;
	struct IntuitionIFace	*IIntuition;
	struct UtilityIFace		*IUtility;
	struct DataTypesIFace	*IDataTypes;
	struct GraphicsIFace	*IGraphics;
	struct DiskfontIFace	*IDiskfont;

	struct Library	*DOSLib;
	struct Library	*IntuitionLib;
	struct Library	*UtilityLib;
	struct Library	*DataTypesLib;
	struct Library	*GraphicsLib;
	struct Library	*SuperClassLib;
	struct Library	*DiskfontLib;
};

#define DOSLib			libBase->DOSLib
#define IntuitionLib	libBase->IntuitionLib
#define UtilityLib		libBase->UtilityLib
#define DataTypesLib	libBase->DataTypesLib
#define GraphicsLib		libBase->GraphicsLib
#define SuperClassLib	libBase->SuperClassLib
#define DiskfontLib		libBase->DiskfontLib

#define IExec		libBase->IExec
#define IDOS		libBase->IDOS
#define IIntuition	libBase->IIntuition
#define IUtility	libBase->IUtility
#define IDataTypes	libBase->IDataTypes
#define IGraphics	libBase->IGraphics
#define IDiskfont	libBase->IDiskfont

#define VTX_RAW 0
#define VTX_EP1 1
#define VTX_EPX 2
#define VTX_VIEWDATA 3

Class *initDTClass (struct ClassBase *libBase);
BOOL freeDTClass (struct ClassBase *libBase, Class *cl);

#define OK (0)
#define NOTOK DTERROR_INVALID_DATA
#define ERROR_EOF DTERROR_NOT_ENOUGH_DATA

#define ReadError(C) ((C == -1) ? IDOS->IoErr() : ERROR_EOF)
#define WriteError(C) IDOS->IoErr()

#endif /* ICO_CLASS_H */
