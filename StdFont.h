
#ifndef STDFONT_H
#define STDFONT_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

#ifdef QT_GUI_LIB

#include <QtGui/QPainter>

#endif /* QT_GUI_LIB */

/* # of colours that text can be displayed in and the symbolic names of */
/* those colours */

#define STDFONT_NUM_COLOURS 4
#define STDFONT_BLACK 0
#define STDFONT_RED 1
#define STDFONT_GREEN 2
#define STDFONT_BLUE 3

class RFont
{
private:

#if defined(__amigaos4__) || defined(QT_GUI_LIB)

	TInt		m_iBaseline;		/**< Distance from the top of char to baseline */

#endif /* defined(__amigaos4__) || defined(QT_GUI_LIB) */

#ifdef __amigaos4__

	LONG		m_alPens[STDFONT_NUM_COLOURS]; /* Array of pens found using IGraphics->ObtainBestPen() */
	struct TextFont	*m_poFont;		/**< Amiga font with which to render */
	struct TextFont *m_poOldFont;	/**< Default system font used for rendering text */

#elif defined(QT_GUI_LIB)

	QPainter	m_oPainter;			/**< Object used for rendering text to the screen */
	QColor		m_oBackground;		/**< Background and text colours at the time */
	QColor		m_oText;			/**< that RFont.Begin() was called */
	QFont		*m_poFont;			/**< Qt font with which to render */

#elif defined(WIN32)

	HDC			m_poDC;				/**< Ptr to temporary DC, if required */
	HFONT		m_poFont;			/**< Windows font with which to render */
	HFONT		m_poOldFont;		/**< Windows font previously selected into window */
	COLORREF	m_oBackground;		/**< Background and text colours at the time */
	COLORREF	m_oText;			/**< that RFont.Begin() was called */

#endif /* WIN32 */

#ifdef _DEBUG

	TBool		m_bBeginCalled;		/**< ETrue if RFont::Begin() has been called */

#endif /* _DEBUG */

	const char	*m_pccName;			/**< The name of the font */
	TBool		m_bHighlight;		/**< ETrue if text will be drawn highlighted, else EFalse */
	TInt		m_aiFontSizes[20];	/**< Array of available font sizes */
	TInt		m_iNumSizes;		/**< Number of valid sizes in m_aiFontSizes */
	TInt		m_iClipWidth;		/**< Number of pixels to draw horizontally before clipping */
	TInt		m_iClipHeight;		/**< Number of pixels to draw vertically before clipping */
	TInt		m_iWidth;			/**< Width of a character in pixels.  Valid only for monospace fonts */
	TInt		m_iHeight;			/**< Height of a character in pixels */
	TInt		m_iXOffset;			/**< Offset from left of window at which to print */
	TInt		m_iYOffset;			/**< Offset from top of window at which to print */
	CWindow		*m_poWindow;		/**< Ptr to window into which to render */

public:

	RFont(CWindow *a_poWindow);

	TInt Open(TInt a_iSize, const char *a_pccName);

	void Close();

	TInt Begin();

	void End();

	void DrawCursor(const char *a_pccText, TInt a_iX, TInt a_iY, TBool a_iDrawCharacter);

	void DrawText(const char *a_pccText, TInt a_iLength, TInt a_iX, TInt a_iY);

	void DrawColouredText(const char *a_pccText, TInt a_iX, TInt a_iY);

	TInt GetNextSize(TInt a_iSize, TBool a_bLarger);

	TInt Width()
	{
		return(m_iWidth);
	}

	TInt Height()
	{
		return(m_iHeight);
	}

	void SetHighlight(TBool a_bHighlight);

	void SetDrawingRect(TInt a_iXOffset, TInt a_iYOffset, TInt a_iWidth, TInt a_iHeight);

#if defined(WIN32) && !defined(QT_GUI_LIB)

	static TInt CALLBACK FontNameProc(ENUMLOGFONTEX *a_poEnumLogFont, NEWTEXTMETRICEX *a_poNewTextMetric, TInt a_iFontType, LPARAM a_oLParam);

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

};

#endif /* ! STDFONT_H */
