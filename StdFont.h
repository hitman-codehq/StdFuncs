
#ifndef STDFONT_H
#define STDFONT_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

#ifdef QT_GUI_LIB

#include <QtGui/QPainter>

#endif /* QT_GUI_LIB */

#define NUM_FONT_COLOURS 4

class RFont
{
private:

#ifndef WIN32

	TInt		m_iBaseline;	/* Distance from the top of char to baseline */

#endif /* ! WIN32 */

#ifdef __amigaos4__

	LONG		m_alPens[NUM_FONT_COLOURS];	/* Array of pens found using IGraphics->ObtainBestPen() */

#elif defined(QT_GUI_LIB)

	QPainter	m_oPainter;		/* Object used for rendering text to the screen */
	QColor		m_oBackground;	/* Background and text colours at the time */
	QColor		m_oText;		/* that RFont.Begin() was called */
	QFont		*m_poFont;		/* Qt font with which to render */

#elif defined(WIN32)

	HDC			m_poDC;			/* Ptr to temporary DC, if required */
	HFONT		m_poFont;		/* Windows font with which to render */
	HFONT		m_poOldFont;	/* Windows font previously selected into window */
	COLORREF	m_oBackground;	/* Background and text colours at the time */
	COLORREF	m_oText;		/* that RFont.Begin() was callsed */

#endif /* WIN32 */

	TBool		m_iHighlight;	/* ETrue if text will be drawn highlighted, else EFalse */
	TInt		m_iClipWidth;	/* Number of pixels to draw horizontally before clipping */
	TInt		m_iWidth;		/* Width of a character in pixels.  Valid only for monospace fonts */
	TInt		m_iHeight;		/* Height of a character in pixels */
	TInt		m_iXOffset;		/* Offset from left of window at which to print */
	TInt		m_iYOffset;		/* Offset from top of window at which to print */
	CWindow		*m_poWindow;	/* Ptr to window into which to render */

public:

	RFont(CWindow *a_poWindow);

	TInt Open();

	void Close();

	TInt Begin();

	void End();

	void DrawCursor(const char *a_pcText, TInt a_iX, TInt a_iY, TBool a_iDrawCharacter);

	void DrawText(const char *a_pcText, TInt a_iLength, TInt a_iX, TInt a_iY);

	void DrawColouredText(const char *a_pcText, TInt a_iX, TInt a_iY);

	TInt Width()
	{
		return(m_iWidth);
	}

	TInt Height()
	{
		return(m_iHeight);
	}

	void SetHighlight(TBool a_iHighlight);

	void SetClipWidth(TInt a_iClipWidth);

	void SetXOffset(TInt a_iXOffset);

	void SetYOffset(TInt a_iYOffset);
};

#endif /* ! STDFONT_H */
