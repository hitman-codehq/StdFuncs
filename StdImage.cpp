
#include "StdFuncs.h"
#include "StdImage.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <datatypes/pictureclass.h>
#include <proto/datatypes.h>

#endif /* __amigaos4__ */

/* Written: Saturday 06-Nov-2010 8:19 am */
/* @param	a_pccFileName The name of the image file to be opened */
/* @return	KErrNone the image was loaded successfully; or */
/*			KErrNotFound if the image could not be found */
/*			KErrNoMemory if not enough memory was available */
/* Loads an image into memory using the underlying operating system's image */
/* loading functions.  This enables the client software to load images and to */
/* access them in a platform independent manner while keeping the size of the */
/* executable to a minimum. */

TInt RStdImage::open(const char *a_pccFileName)
{

#ifdef __amigaos4__

	TInt RetVal;
	struct BitMapHeader *BitMapHeader;
	struct gpLayout Layout;

	/* Attempt to load the image using the datatypes system, using "extended mode" that will ensure */
	/* that the highest possible number of colours are used and passing in a ptr to the screen the */
	/* root window is opened on so that the colour mapping is correct.  No I don't understand it either! */

	m_poBitMapObj = IDataTypes->NewDTObject(a_pccFileName, PDTA_Screen, CWindow::GetRootWindowScreen(),
		PDTA_DestMode, PMODE_V43, TAG_DONE);

	if (m_poBitMapObj)
	{
		/* Assume failure */

		RetVal = KErrNoMemory;

		/* Perform the datatype layout method so that the bitmap data is made available */

		Layout.MethodID = DTM_PROCLAYOUT;
		Layout.gpl_GInfo = NULL;
		Layout.gpl_Initial = TRUE;
		IDataTypes->DoDTMethodA(m_poBitMapObj, NULL, NULL, (Msg) &Layout);

		/* And get a ptr to the bitmap data in the form of a BitMap so that it can be used by */
		/* client code */

		if (IDataTypes->GetDTAttrs(m_poBitMapObj, PDTA_BitMap, &m_poBitMap, TAG_DONE) == 1)
		{
			/* Also get the width and height of the BitMap, given that this is not easily able */
			/* to be ascertained from the old graphics.library BitMap structure */

			if (IDataTypes->GetDTAttrs(m_poBitMapObj, PDTA_BitMapHeader, &BitMapHeader, TAG_DONE) == 1)
			{
				/* Signal success */

				RetVal = KErrNone;

				m_iWidth = BitMapHeader->bmh_Width;
				m_iHeight = BitMapHeader->bmh_Height;
			}
			else
			{
				Utils::info("RStdImage::open() => Unable to determine size of image");
			}
		}
		else
		{
			Utils::info("RStdImage::open() => Unable to obtain image BitMap");
		}
	}
	else
	{
		RetVal = KErrNotFound;

		Utils::info("RStdImage::open() => Unable to open image \"%s\"", a_pccFileName);
	}

	return(RetVal);

#elif defined(__unix__)

	// TODO: CAW - Implement and remove warning prevention cast
	(void) a_pccFileName;

	return(KErrNotFound);

#else /* ! __unix__ */

	m_poBitmap = (HBITMAP) LoadImage(GetModuleHandle(NULL), a_pccFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	return((m_poBitmap) ? KErrNone : KErrNotFound);

#endif /* ! __unix__ */

}

/* Written: Saturday 06-Nov-2010 8:36 am */
/* Closes a window previously opened with open(). */

void RStdImage::close()
{

#ifdef __amigaos4__

	/* If any datatype object has been created, destroy it */

	if (m_poBitMapObj)
	{
		IDataTypes->DisposeDTObject(m_poBitMapObj);
		m_poBitMapObj = NULL;
		m_poBitMap = NULL;
	}

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	/* If any bitmap object has been created, destroy it */

	if (m_poBitmap)
	{
		DEBUGCHECK(DeleteObject(m_poBitmap), "RStdImage::close() => Unable to delete bitmap object");
		m_poBitmap = NULL;
	}

#endif /* ! QT_GUI_LIB */

}
