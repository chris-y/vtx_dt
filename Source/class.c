/*
 * videotex.datatype - Chris Young
 */

#include "class.h"

#define RGB8to32(RGB) (((uint32)(RGB) << 24)|((uint32)(RGB) << 16)|((uint32)(RGB) << 8)|((uint32)(RGB)))

uint32 ClassDispatch (Class *cl, Object *o, Msg msg);

void overlaytext(long xoffset,long yoffset,UBYTE text,ULONG pix,ULONG pix2,struct TextFont *tfont,long width,long height,UBYTE *iconimg,Class *cl)
{
	struct ClassBase *libBase = (struct ClassBase *)cl->cl_UserData;
	UBYTE *tchardata;
	int a,b,z,charwanted='Q',start,pchar=0,startp;
	UBYTE tc;
	UWORD *clocdata;
	UWORD cloc,clen;
	long x=0,i=0,c=0;

	if((yoffset+(tfont->tf_YSize)>height) || (xoffset+8>width)) return;


//	for(c=0;c<strlen(text);c++)
//	{
		i=xoffset+(yoffset*width);

		charwanted = text;

		charwanted = charwanted - tfont->tf_LoChar;

		if(charwanted > tfont->tf_HiChar)
			charwanted = tfont->tf_HiChar;

		tchardata = tfont->tf_CharData;

		clocdata = tfont->tf_CharLoc;

// character number * 2

		cloc = clocdata[charwanted*2]; // THIS IS IN PIXELS
		clen = clocdata[1+charwanted*2];
		startp = cloc%8;

		xoffset=xoffset+clen;

		for(a=0;a<tfont->tf_YSize;a++) // height
		{
			b=cloc/8;
			start=startp;

			pchar=0;

			while(pchar<clen)
			{
				tc=tchardata[b];

				z=8;

				if(start>0)
				{
					tc = tc<<start;
					z = 8-start;
				}

				if((clen-pchar)<z) z=(clen-pchar);

				for(x=0;x<z;x++)
				{
					if(i >= width * height) return;

					if(tc & 0x80)
					{
						iconimg[i]=pix;
					}
					else
					{
						iconimg[i]=pix2;
					}
//if(i>76799) IExec->DebugPrintF("write posn %ld\n",i);

					i++;
					pchar++;

					if(pchar>=clen)
					{
						i=i+width-clen;
					}
					tc = tc<<1;
				}
				b++;
				start=0;
			}
			cloc=cloc+(tfont->tf_Modulo*8);
			pchar=0;
		}
}

Class *initDTClass (struct ClassBase *libBase) {
	Class *cl;
	SuperClassLib = IExec->OpenLibrary("datatypes/picture.datatype", 52);
	if (SuperClassLib) {
		cl = IIntuition->MakeClass(libBase->libNode.lib_Node.ln_Name, PICTUREDTCLASS, NULL, 0, 0);
		if (cl) {
			cl->cl_Dispatcher.h_Entry = (HOOKFUNC)ClassDispatch;
			cl->cl_UserData = (uint32)libBase;
			IIntuition->AddClass(cl);
		} else {
			IExec->CloseLibrary(SuperClassLib);
		}
	}
	return cl;
}

BOOL freeDTClass (struct ClassBase *libBase, Class *cl) {
	BOOL res = TRUE;
	if (cl) {
		res = IIntuition->FreeClass(cl);
		if (res) {
			IExec->CloseLibrary(SuperClassLib);
		}
	}
	return res;
}

static int32 WriteICO (Class *cl, Object *o, struct dtWrite *msg);
static int32 ConvertICO (Class *cl, Object *o, BPTR file, struct BitMapHeader *bmh, uint32 index, uint32 *total);
static int32 GetICO (Class *cl, Object *o, struct TagItem *tags);

uint32 ClassDispatch (Class *cl, Object *o, Msg msg) {
	struct ClassBase *libBase = (struct ClassBase *)cl->cl_UserData;
	uint32 ret;

	switch (msg->MethodID) {

		case OM_NEW:
			ret = IIntuition->IDoSuperMethodA(cl, o, msg);
			if (ret) {
				int32 error;
				error = GetICO(cl, (Object *)ret, ((struct opSet *)msg)->ops_AttrList);
				if (error != OK) {
					IIntuition->ICoerceMethod(cl, (Object *)ret, OM_DISPOSE);
					ret = (uint32)NULL;
					IDOS->SetIoErr(error);
				}
			}
			break;

/*
		case DTM_WRITE:
			// check if dt's native format should be used
			if (((struct dtWrite *)msg)->dtw_Mode == DTWM_RAW) {
				int32 error;
				error = WriteICO(cl, o, (struct dtWrite *)msg);
				if (error == OK) {
					ret = TRUE;
				} else {
					IDOS->SetIoErr(error);
					ret = FALSE;
				}
				break;
			}
*/
			/* fall through and let superclass deal with it */

		default:
			ret = IIntuition->IDoSuperMethodA(cl, o, msg);
			break;

	}

	return ret;
}

static int32 WriteICO (Class *cl, Object *o, struct dtWrite *msg) {
	return ERROR_NOT_IMPLEMENTED;
}

static int32 ConvertICO (Class *cl, Object *o, BPTR file, struct BitMapHeader *bmh, uint32 index, uint32 *total)
{
	struct ClassBase *libBase = (struct ClassBase *)cl->cl_UserData;
	int32 status, error = OK;
	uint32 fileoffs;
	int32 numcolors;
	uint32 num;
	int size;
	int32 level = 0;
	struct ColorRegister *cmap = NULL;
	uint32 *cregs = NULL;		
	UBYTE *tmp;
	LONG vtxchar = 0;
	UBYTE *bitmap;
	long row=0,height=0,width=0,charheight=24,col=0,charwidth=40;
	int i=0;
	struct TextAttr tattr = {"MtA_US-ASCII.font",10,FS_NORMAL,0}; //,NULL};
	struct TextAttr tattrdbl = {"MtA_US-ASCII.font",20,FS_NORMAL,0}; //,NULL};
	//struct TagItem texttags[2];
	struct TextFont *tfont,*tfontdbl,*font;
	struct ColorRegister *ocmap = NULL;
	uint32 *ocregs = NULL;
	int fcol=7,bcol=0,textadd=0,gfx=0,gfxmode=0,heldchar=32,hold=0,dbl=0;
	struct FileInfoBlock *fib = NULL;
	uint32 coltemp;
	uint32 vtxformat = VTX_RAW;
	int esc = 0;
	char header[10];

	IDOS->ChangeFilePosition(file,0,OFFSET_BEGINNING);

	if(IDOS->Read(file, &header, 7))
	{
		if(header[0] == 'J' && header[1] == 'W' && header[2] == 'C' &&
			header[4] == 0x00 && header[5] == 0x00)
		{
			vtxformat = VTX_EPX;
			if(total) *total = header[3];
		}

		if(header[0] == 0xFE && header[1] == 0x01 && header[2] == 0x09 &&
			header[3] == 0x00 && header[4] == 0x00 && header[5] == 0x00)
		{
			vtxformat = VTX_EP1;
			if(total) *total = 1;
		}
	}

	if(vtxformat == VTX_RAW)
	{
		fib = IDOS->AllocDosObject(DOS_FIB,NULL);
		IDOS->ExamineFH(file,fib);
		if(total) *total = fib->fib_Size / 960;
		IDOS->FreeDosObject(DOS_FIB,fib);
	}

	if(total)
	{
		if (index >= *total) return DTERROR_NOT_AVAILABLE;
	}

	switch(vtxformat)
	{
		case VTX_RAW:
			IDOS->ChangeFilePosition(file,index*(charheight * charwidth),OFFSET_BEGINNING);
		break;

		case VTX_EPX:
			charheight = 25;
			IDOS->ChangeFilePosition(file,12 + (index*((charheight * charwidth) + 8)),OFFSET_BEGINNING);
		break;

		case VTX_EP1:
			charheight = 25;
			IDOS->ChangeFilePosition(file,6,OFFSET_BEGINNING);
		break;
	}

	height = charheight*10;
	width = charwidth*8;

	// this is a workaround for a bug in picture.datatype 52.1

	IDataTypes->GetDTAttrs(o,
		PDTA_NumColors,&coltemp,
		TAG_END);

	if(coltemp>0) {
		IDataTypes->SetDTAttrs(o, NULL, NULL,
			PDTA_NumColors,0,
			TAG_END);
	}

	// end workaround

	IDataTypes->SetDTAttrs(o, NULL, NULL,
		PDTA_NumColors,8,
		TAG_END);

	IDataTypes->GetDTAttrs(o,
		PDTA_ColorRegisters,	&cmap,
		PDTA_CRegs,				&cregs,
		TAG_END); // == 2

        bitmap = (unsigned char *) IExec->AllocVec(height*width,MEMF_CLEAR);
		if(!bitmap) return ERROR_NO_FREE_STORE;

//IExec->DebugPrintF("bufsize = %ld\n",height*width);

/*
		texttags[0].ti_Tag = TA_CharSet;
		texttags[0].ti_Data = 3; //59;
		texttags[1].ti_Tag = TAG_END;
		tattr.tta_Tags = texttags;
		tattrdbl.tta_Tags = texttags;
*/

	tfont = IDiskfont->OpenDiskFont((struct TextAttr *)&tattr);
	tfontdbl = IDiskfont->OpenDiskFont((struct TextAttr *)&tattrdbl);

	if(!tfont || !tfontdbl) {
		IExec->DebugPrintF("[videotex.datatype] Cannot open MtA.font\n");
		return ERROR_NOT_IMPLEMENTED;
	}

        for(row=0;row<charheight; row++)
        {
		gfx=0;
		bcol=0;
		fcol=7;
		gfxmode=0;
		heldchar=32;
		hold=0;

		if(dbl==1) {
			IDOS->ChangeFilePosition(file,40,OFFSET_CURRENT);
			row++;
			if(row>=charheight) break;
		}

		font=tfont;
		dbl=0;

		for(col=0; col<charwidth; col++) {
			vtxchar = IDOS->FGetC(file);

			// need to write our bitmap here

			if(vtxchar == -1) break;
			vtxchar &= 0x7F;

			if(vtxchar == 0x0D) break;

			if(vtxchar==0x1B) {
				vtxchar = IDOS->FGetC(file);
				if(vtxchar == -1) break;
				vtxchar &= 0x7F;

				esc = 1; // ignore non-escape codes from now on

				if(vtxchar>=0x40 && vtxchar<=0x47) {
					fcol=vtxchar-0x40;
					gfx=0;
					heldchar=32;
					vtxchar=heldchar;
				}

				if(vtxchar>=0x50 && vtxchar<=0x57) {
					fcol=vtxchar-0x50;
					gfx=1;
					vtxchar=heldchar;
				}

				if(vtxchar==0x59) {
					gfxmode=0;
					vtxchar=heldchar;
				}

				if(vtxchar==0x5A) {
					gfxmode=64;
					vtxchar=heldchar;
				}

				if(vtxchar==0x5C) {
					bcol=0;
					vtxchar=heldchar;
				}

				if(vtxchar==0x5D) {
					bcol=fcol;
					vtxchar=heldchar;
				}

				if(vtxchar==0x4C) {
					font=tfont;
					vtxchar=heldchar;
				}

				if(vtxchar==0x4D) {
					font=tfontdbl;
					vtxchar=heldchar;
					dbl=1;
				}

				if(vtxchar==0x48 || vtxchar == 0x49 || vtxchar==0x58) {
					// flash, steady and conceal - not implemented.
					vtxchar=heldchar;
				}

				if(vtxchar==0x5E) {
					hold=1;
					heldchar=32;
					vtxchar=heldchar;
				}

				if(vtxchar==0x5F) {
					hold=0;
					heldchar=32;
					vtxchar=heldchar;
				}
			}

			if(esc == 0) {
				if(vtxchar>=0x00 && vtxchar<=0x07) {
					fcol=vtxchar;
					gfx=0;
					heldchar=32;
					vtxchar=heldchar;
				}

				if(vtxchar>=0x10 && vtxchar<=0x17) {
					fcol=vtxchar-0x10;
					gfx=1;
					vtxchar=heldchar;
				}

				if(vtxchar==0x19) {
					gfxmode=0;
					vtxchar=heldchar;
				}

				if(vtxchar==0x1A) {
					gfxmode=64;
					vtxchar=heldchar;
				}

				if(vtxchar==0x1C) {
					bcol=0;
					vtxchar=heldchar;
				}

				if(vtxchar==0x1D) {
					bcol=fcol;
					vtxchar=heldchar;
				}

				if(vtxchar==0x0C) {
					font=tfont;
					vtxchar=heldchar;
				}

				if(vtxchar==0x0D) {
					font=tfontdbl;
					vtxchar=heldchar;
					dbl=1;
				}

				if(vtxchar==0x08 || vtxchar == 0x09 || vtxchar==0x18) {
					// flash, steady and conceal - not implemented.
					vtxchar=heldchar;
				}

				if(vtxchar==0x1E) {
					hold=1;
					heldchar=32;
					vtxchar=heldchar;
				}

				if(vtxchar==0x1F) {
					hold=0;
					heldchar=32;
					vtxchar=heldchar;
				}
			}

			if(gfx==1) {
				if(vtxchar>=0x20 && vtxchar<=0x3F) {
					textadd=96+gfxmode;
				} else if(vtxchar>=0x60 && vtxchar<=0x7F) {
					textadd=64+gfxmode;
				} else {
					textadd=0;
				}

				if(hold==1) heldchar=vtxchar;
			}

			overlaytext(col*8,row*10,vtxchar+textadd,fcol,bcol,font,width,height,bitmap,cl);
			textadd=0;

			if((font==tfont) && ((row+1) < charheight)) overlaytext(col*8,(row+1)*10,32,fcol,bcol,font,width,height,bitmap,cl); // write the same colour data underneath in case of double-height codes.
		}
	}

	IGraphics->CloseFont(tfont);
	IGraphics->CloseFont(tfontdbl);

// these are the colours

	ocmap=cmap;

	ocmap->red = 0;
	ocmap->green = 0;
	ocmap->blue = 0;
	ocmap++;

	ocmap->red = 255;
	ocmap->green = 0;
	ocmap->blue = 0;
	ocmap++;

	ocmap->red = 0;
	ocmap->green = 255;
	ocmap->blue = 0;
	ocmap++;

	ocmap->red = 255;
	ocmap->green = 255;
	ocmap->blue = 0;
	ocmap++;

	ocmap->red = 0;
	ocmap->green = 0;
	ocmap->blue = 255;
	ocmap++;

	ocmap->red = 255;
	ocmap->green = 0;
	ocmap->blue = 255;
	ocmap++;

	ocmap->red = 0;
	ocmap->green = 255;
	ocmap->blue = 255;
	ocmap++;

	ocmap->red = 255;
	ocmap->green = 255;
	ocmap->blue = 255;
//	ocmap++;

	ocmap = cmap;

	for(i=0;i<8;i++)
	{
		*cregs++ = RGB8to32(ocmap->red);
		*cregs++ = RGB8to32(ocmap->green);
		*cregs++ = RGB8to32(ocmap->blue);
		ocmap++;
	}
/**********/

	bmh->bmh_Width = width;
	bmh->bmh_Height = height;
	bmh->bmh_Depth = 3;

//	uint32 bpr = (bmh->bmh_Width + 3) & ~3;

	IDataTypes->SetDTAttrs(o, NULL, NULL,
	DTA_NominalHoriz,	bmh->bmh_Width,
	DTA_NominalVert,	bmh->bmh_Height,
	PDTA_SourceMode,	PMODE_V43,
	DTA_ErrorLevel,	&level,
	DTA_ErrorNumber,	&error,
	TAG_END);

	IIntuition->IDoSuperMethod(cl, o,
	PDTM_WRITEPIXELARRAY,bitmap, PBPAFMT_LUT8,
	bmh->bmh_Width, 0, 0, bmh->bmh_Width, bmh->bmh_Height);

	IExec->FreeVec(bitmap);

	return error;
}

static int32 GetICO (Class *cl, Object *o, struct TagItem *tags) {
	struct ClassBase *libBase = (struct ClassBase *)cl->cl_UserData;
	struct BitMapHeader *bmh = NULL;
	char *filename;
	int32 srctype;
	int32 error = ERROR_OBJECT_NOT_FOUND;
	BPTR file = (BPTR)NULL;
	uint32 whichpic, *numpics;

	filename = (char *)IUtility->GetTagData(DTA_Name, (uint32)"Untitled", tags);
	whichpic = IUtility->GetTagData(PDTA_WhichPicture, 0, tags);
	numpics = (uint32 *)IUtility->GetTagData(PDTA_GetNumPictures, (uint32)NULL, tags);

	IDataTypes->GetDTAttrs(o,
		PDTA_BitMapHeader,	&bmh,
		DTA_Handle,			&file,
		DTA_SourceType,		&srctype,
		TAG_END);

	/* Do we have everything we need? */
	if (bmh && file && srctype == DTST_FILE) {
		error = ConvertICO(cl, o, file, bmh, whichpic, numpics);
		if (error == OK) {

			IDataTypes->SetDTAttrs(o, NULL, NULL,
				DTA_ObjName,IDOS->FilePart(filename),
				TAG_END);

		}
	}
	return error;
}
