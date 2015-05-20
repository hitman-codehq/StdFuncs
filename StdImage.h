
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
private:

	TInt			m_iWidth;		/**< Width and height of the image, as determined from */
	TInt			m_iHeight;		/**< the image file upon loading */

#ifdef __amigaos4__

	Object			*m_poBitMapObj;	/**< Datatype object representing the loaded image */
	struct BitMap	*m_poBitMap;	/**< BitMap data extracted from the bitmap datatype */

#elif defined(WIN32)

	HBITMAP			m_poBitmap;		/**< Handle to Windows bitmap representing the loaded image */

#endif /* WIN32 */

public:

	RStdImage()
	{
		m_iWidth = m_iHeight = 0;

#ifdef __amigaos4__

		m_poBitMapObj = NULL;
		m_poBitMap = NULL;

#elif defined(WIN32)

		m_poBitmap = NULL;

#endif /* WIN32 */

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

#elif defined(WIN32)

	HBITMAP Bitmap()
	{
		return(m_poBitmap);
	}

#endif /* WIN32 */

};

#endif /* ! STDIMAGE_H */

