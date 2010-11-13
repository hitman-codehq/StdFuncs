
#ifndef STDIMAGE_H
#define STDIMAGE_H

#ifdef __amigaos4__

#include <intuition/classes.h>

#endif /* __amigaos4__ */

/* A class to allow programs to load images without having to be concerned about their */
/* type.  The range of images supported is platform dependent given that this class is */
/* essentially a wrapper around whatever platform specific image loading functions are */
/* present on the host platform. */

class RStdImage
{
	TInt			m_iWidth;		/* Width and height of the image, as determined from */
	TInt			m_iHeight;		/* the image file upon loading */

#ifdef __amigaos4__

	Object			*m_poBitMapObj;	/* Datatype object representing the loaded image */
	struct BitMap	*m_poBitMap;	/* BitMap data extracted from the bitmap datatype */

#else /* ! __amigaos4__ */

	HBITMAP			m_poBitmap;		/* Handle to Windows bitmap representing the loaded image */

#endif /* ! __amigaos4__ */

public:

	RStdImage()
	{
		m_iWidth = m_iHeight = 0;

#ifdef __amigaos4__

		m_poBitMapObj = NULL;
		m_poBitMap = NULL;

#else /* ! __amigaos4__ */

		m_poBitmap = NULL;

#endif /* ! __amigaos4__ */

	}

	TInt Open(const char *a_pccFileName);

	void Close();

	TInt Width()
	{
		return(m_iWidth);
	}

	TInt Height()
	{
		return(m_iHeight);
	}

#ifdef __amigaos4__

	struct BitMap *BitMap()
	{
		return(m_poBitMap);
	}

#else /* ! __amigaos4__ */

	HBITMAP Bitmap()
	{
		return(m_poBitmap);
	}

#endif /* ! __amigaos4__ */

};

#endif /* ! STDIMAGE_H */

