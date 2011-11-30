
#ifndef STDFONT_H
#define STDFONT_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

class RFont
{
private:

#ifdef __amigaos4__

	TInt		m_iBaseline;	/* Distance from the top of char to baseline */

#else /* ! __amigaos4__ */

	HDC			m_poDC;			/* Ptr to temporary DC, if required */
	HFONT		m_poFont;		/* Win32 font with which to render */
	HFONT		m_poOldFont;	/* Win32 font previously selected into window */
	COLORREF	m_oBackground;	/* Background colour and text colours at the */
	COLORREF	m_oText;		/* time that RFont.Open() was callsed */

#endif /* ! __amigaos4__ */

	TBool		m_iHighlight;	/* ETrue if text will be drawn highlighted, else EFalse */
	TInt		m_iClipWidth;	/* Nmber of pixels to draw horizontally before clipping */
	TInt		m_iWidth;		/* Width of a character in pixels.  Valid only for monospace fonts */
	TInt		m_iHeight;		/* Height of a character in pixels */
	TInt		m_iYOffset;		/* Offset from top of window at which to print */
	CWindow		*m_poWindow;	/* Ptr to window into which to render */

public:

	RFont(CWindow *a_poWindow);

	TInt Open();

	void Close();

	void DrawCursor(const char *a_pcText, TInt a_iX, TInt a_iY, TBool a_iDrawCharacter);

	void DrawText(const char *a_pcText, TInt a_iLength, TInt a_iX, TInt a_iY);

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

	void SetYOffset(TInt a_iYOffset);
};

#endif /* ! STDFONT_H */
