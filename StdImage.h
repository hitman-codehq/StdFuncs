
#ifndef STDIMAGE_H
#define STDIMAGE_H

/** @file */

#ifdef __amigaos__

#include <intuition/classes.h>

#elif defined(QT_GUI_LIB)

class QPixmap;

#endif /* QT_GUI_LIB */

/* A class to allow programs to load images without having to be concerned about their */
/* type.  The range of images supported is platform dependent given that this class is */
/* essentially a wrapper around whatever platform specific image loading functions are */
/* present on the host platform. */

class RStdImage
{
private:

	TInt			m_iWidth;		/**< Width and height of the image, as determined from */
	TInt			m_iHeight;		/**< the image file upon loading */

#ifdef __amigaos__

	Object			*m_poBitMapObj;	/**< Datatype object representing the loaded image */
	struct BitMap	*m_poBitMap;	/**< BitMap data extracted from the bitmap datatype */

#elif defined(QT_GUI_LIB)

	QPixmap			*m_poBitmap;	/** Pointer to Qt bitmap representing the loaded image */

#elif defined(WIN32)

	HBITMAP			m_poBitmap;		/**< Handle to Windows bitmap representing the loaded image */

#endif /* WIN32 */

public:

	RStdImage()
	{
		m_iWidth = m_iHeight = 0;

#ifdef __amigaos__

		m_poBitMapObj = NULL;
		m_poBitMap = NULL;

#elif defined(QT_GUI_LIB) || defined(WIN32)

		m_poBitmap = NULL;

#endif /* defined(QT_GUI_LIB) || defined(WIN32) */

	}

	TInt open(const char *a_pccFileName);

	void close();

	TInt Width()
	{
		return(m_iWidth);
	}

	TInt Height()
	{
		return(m_iHeight);
	}

#ifdef __amigaos__

	const struct BitMap *BitMap() const
	{
		return(m_poBitMap);
	}

#elif defined(QT_GUI_LIB)

	const QPixmap *Bitmap() const
	{
		return(m_poBitmap);
	}

#elif defined(WIN32)

	const HBITMAP Bitmap() const
	{
		return(m_poBitmap);
	}

#endif /* WIN32 */

};

#endif /* ! STDIMAGE_H */
