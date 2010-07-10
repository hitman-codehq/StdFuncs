
#include "StdFuncs.h"
#include "StdFont.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/graphics.h>

#endif /* __amigaos4__ */

/* Written: Sunday 31-May-2010 1:41 pm */

RFont::RFont(CWindow *a_poWindow)
{
	ASSERTM(a_poWindow, "RFont::RFont() => Window handle must not be NULL");

#ifndef __amigaos4__

	m_poFont = m_poOldFont = NULL;

#endif /* ! __amigaos4__ */

	m_poWindow = a_poWindow;
}

/* Written: Sunday 31-May-2010 3:38 pm */

TInt RFont::Open()
{
	TInt RetVal;

#ifdef __amigaos4__

	ASSERTM(m_poWindow, "RFont::Open() => Window handle not set");

	/* RFont::Open() cannow fail on the Amiga */

	RetVal = KErrNone;

	/* Determine the baseline & height of the font from the window */

	m_iBaseline = m_poWindow->m_poWindow->IFont->tf_Baseline;
	m_iHeight = m_poWindow->m_poWindow->IFont->tf_YSize;

#else /*  ! __amigaos4__ */

	int Height;
	TEXTMETRIC TextMetric;

	ASSERTM(m_poWindow->m_poDC, "RFont::Open() => Function should only be called from CWindow::Draw");

	/* Convert the font size from the desired point size to pixels per inch and attempt to open a font */
	/* that uses that size */

	Height = -MulDiv(10, GetDeviceCaps(m_poWindow->m_poDC, LOGPIXELSY), 72);

	if ((m_poFont = CreateFont(Height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, (FF_MODERN | FIXED_PITCH), "Courier")) != NULL)
	{
		DEBUGCHECK(SelectObject(m_poWindow->m_poDC, m_poFont));
	}
	else
	{
		Utils::Info("RFont::Open() => Unable to open requested font, using default");
	}

	/* Determine the height of the font from the device context */

	if (GetTextMetrics(m_poWindow->m_poDC, &TextMetric))
	{
		RetVal = KErrNone;

		m_iHeight = TextMetric.tmHeight;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#endif /*  ! __amigaos4__ */

	return(RetVal);
}

/* Written: Monday 05-Jul-2010 7:13 am */

void RFont::Close()
{

#ifndef __amigaos4__

	if (m_poOldFont)
	{
		DEBUGCHECK(SelectObject(m_poWindow->m_poDC, m_poOldFont));
		m_poOldFont = NULL;
	}

	if (m_poFont)
	{
		DEBUGCHECK(DeleteObject(m_poFont));
		m_poFont = NULL;
	}

#endif /* ! __amigaos4__ */

}

/* Written: Tuesday 08-Jun-2010 6:22 am */

void RFont::DrawCursor(const char *a_pcText, TInt a_iX, TInt a_iY)
{
	ASSERTM(m_poWindow, "RFont::DrawCursor() => Window handle not set");

	char Cursor[1] = { '@' };

#ifdef __amigaos4__

	int Width;

	(void) a_pcText;

	/* Move to the position at which to print, taking into account the left and top border sizes, */
	/* the height of the current font and the baseline of the font, given that IGraphics->Text() */
	/* routine prints at the baseline position, not the top of the font */

	// TODO: CAW - This should work with proportional fonts
	Width = m_poWindow->m_poWindow->IFont->tf_XSize;

	IGraphics->Move(m_poWindow->m_poWindow->RPort, (m_poWindow->m_poWindow->BorderLeft + (a_iX * Width)),
		(m_poWindow->m_poWindow->BorderTop + (a_iY * m_iHeight) + m_iBaseline));

	/* Set the background and foreground pens, in case they have been changed since the last call */

	IGraphics->SetAPen(m_poWindow->m_poWindow->RPort, 1);
	IGraphics->SetBPen(m_poWindow->m_poWindow->RPort, 0);

	/* And draw the text passed in */

	IGraphics->Text(m_poWindow->m_poWindow->RPort, Cursor, 1);

#else /* ! __amigaos4__ */

	SIZE Size;

	/* Given the X position of the cursor, determine the number of pixels between the left of */
	/* the window and the position at which the cursor is to be displayed, and use this and the */
	/* font height to display the cursor in the correct position */

	if (GetTextExtentPoint32(m_poWindow->m_poDC, a_pcText, a_iX, &Size))
	{
		TextOut(m_poWindow->m_poDC, Size.cx, (a_iY * m_iHeight), Cursor, 1);
	}

#endif /* ! __amigaos4__ */

}

/* Written: Sunday 09-May-2010 6:57 pm */

void RFont::DrawText(const char *a_pcText, TInt a_iLength, TInt a_iY, TBool a_iHighlight)
{
	ASSERTM(m_poWindow, "RFont::DrawText() => Window handle not set");

#ifdef __amigaos4__

	/* Move to the position at which to print, taking into account the left and top border sizes, */
	/* the height of the current font and the baseline of the font, given that IGraphics->Text() */
	/* routine prints at the baseline position, not the top of the font */

	IGraphics->Move(m_poWindow->m_poWindow->RPort, m_poWindow->m_poWindow->BorderLeft,
		(m_poWindow->m_poWindow->BorderTop + (a_iY * m_iHeight) + m_iBaseline));

	/* Set the background and foreground pens, in case they have been changed since the last call */

	IGraphics->SetAPen(m_poWindow->m_poWindow->RPort, 1);
	IGraphics->SetBPen(m_poWindow->m_poWindow->RPort, 0);

	/* And draw the text passed in */

	IGraphics->Text(m_poWindow->m_poWindow->RPort, a_pcText, a_iLength);

#else /* ! __amigaos4__ */

	static TBool Switched = EFalse;

	// TODO: CAW - Do this properly
	COLORREF Background = GetBkColor(m_poWindow->m_poDC);
	COLORREF Text = GetTextColor(m_poWindow->m_poDC);

	if ((a_iHighlight) && (!(Switched)))
	{
		Switched = ETrue;
		SetBkColor(m_poWindow->m_poDC, Text);
		SetTextColor(m_poWindow->m_poDC, Background);
	}

	if ((!(a_iHighlight)) && (Switched))
	{
		Switched = EFalse;
		SetBkColor(m_poWindow->m_poDC, Text);
		SetTextColor(m_poWindow->m_poDC, Background);
	}

	TextOut(m_poWindow->m_poDC, 0, (a_iY * m_iHeight), a_pcText, a_iLength);

#endif /* ! __amigaos4__ */

}

/* Written: Sunday 31-May-2010 2:36 pm */

TInt RFont::Height()
{
	return(m_iHeight);
}
