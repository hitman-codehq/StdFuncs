
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

#ifdef __amigaos4__

	m_iBaseline = 0;

#else /* ! __amigaos4__ */

	m_poDC = NULL;
	m_poFont = m_poOldFont = NULL;

#endif /* ! __amigaos4__ */

	m_iHighlight = EFalse;
	m_iYOffset = m_iWidth = m_iHeight = 0;
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
	m_iWidth = m_poWindow->m_poWindow->IFont->tf_XSize;
	m_iHeight = m_poWindow->m_poWindow->IFont->tf_YSize;

#else /*  ! __amigaos4__ */

	int Height;
	TEXTMETRIC TextMetric;

	/* Assume failure */

	RetVal = KErrGeneral;

	/* If no DC is specified for the window, create a temporary one for use by the class */

	if (m_poWindow->m_poDC == NULL)
	{
		m_poDC = m_poWindow->m_poDC = GetDC(m_poWindow->m_poWindow);
	}

	if (m_poWindow->m_poDC)
	{
		/* Convert the font size from the desired point size to pixels per inch and attempt to open a font */
		/* that uses that size */

		Height = -MulDiv(10, GetDeviceCaps(m_poWindow->m_poDC, LOGPIXELSY), 72);

		if ((m_poFont = CreateFont(Height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, (FF_MODERN | FIXED_PITCH), "Courier")) != NULL)
		{
			DEBUGCHECK(SelectObject(m_poWindow->m_poDC, m_poFont), "RFont::Open() => Unable to select font into device context");
		}
		else
		{
			Utils::Info("RFont::Open() => Unable to open requested font, using default");
		}

		/* Determine the width & height of the font from the device context */

		if (GetTextMetrics(m_poWindow->m_poDC, &TextMetric))
		{
			RetVal = KErrNone;

			m_iWidth = TextMetric.tmAveCharWidth;
			m_iHeight = TextMetric.tmHeight;
		}

		/* Save the background and text colours for l8r use */

		m_oBackground = GetBkColor(m_poWindow->m_poDC);
		m_oText = GetTextColor(m_poWindow->m_poDC);
	}
	else
	{
		Utils::Info("RFont::Open() => Unable to create temporary DC");
	}

#endif /*  ! __amigaos4__ */

	return(RetVal);
}

/* Written: Monday 05-Jul-2010 7:13 am */

void RFont::Close()
{
	/* Set the text and background colours back to their default as these are held by the */
	/* operating system and will persist across instances of the RFont class */

	SetHighlight(EFalse);

#ifndef __amigaos4__

	if (m_poOldFont)
	{
		DEBUGCHECK(SelectObject(m_poWindow->m_poDC, m_poOldFont), "RFont::Close() => Unable to unselect font from device context");
		m_poOldFont = NULL;
	}

	if (m_poFont)
	{
		DEBUGCHECK(DeleteObject(m_poFont), "RFont::Close() => Unable to delete font object");
		m_poFont = NULL;
	}

	if (m_poDC)
	{
		ReleaseDC(m_poWindow->m_poWindow, m_poDC);
		m_poDC = m_poWindow->m_poDC = NULL;
	}

#endif /* ! __amigaos4__ */

}

/* Written: Tuesday 08-Jun-2010 6:22 am */

void RFont::DrawCursor(const char *a_pcText, TInt a_iX, TInt a_iY, TBool a_iDrawCharacter)
{
	ASSERTM(m_poWindow, "RFont::DrawCursor() => Window handle not set");

	char Space[1] = { ' ' };
	const char *Cursor;

	/* Invert the current highlight state before drawing the cursor and draw the letter under */
	/* the cursor.  This will cause it to be highlighted.  We toggle the highlight rather than */
	/* use ETrue and EFalse so that the cursor works within highlighted blocks of text as well */

	SetHighlight(!(m_iHighlight));

	/* Draw the text or a space instead as requested */

	Cursor = (a_iDrawCharacter) ? &a_pcText[a_iX] : Space;

#ifdef __amigaos4__

	int Width;

	/* Move to the position at which to print, taking into account the left and top border sizes, */
	/* the height of the current font and the baseline of the font, given that IGraphics->Text() */
	/* routine prints at the baseline position, not the top of the font */

	// TODO: CAW - This should work with proportional fonts
	Width = m_poWindow->m_poWindow->IFont->tf_XSize;

	IGraphics->Move(m_poWindow->m_poWindow->RPort, (m_poWindow->m_poWindow->BorderLeft + (a_iX * Width)),
		(m_poWindow->m_poWindow->BorderTop + (a_iY * m_iHeight) + m_iBaseline));

	/* And draw the cursor! */

	IGraphics->Text(m_poWindow->m_poWindow->RPort, Cursor, 1);

#else /* ! __amigaos4__ */

	SIZE Size;

	/* Given the X position of the cursor, determine the number of pixels between the left of */
	/* the window and the position at which the cursor is to be displayed, and use this and the */
	/* font height to display the cursor in the correct position */

	if (GetTextExtentPoint32(m_poWindow->m_poDC, a_pcText, a_iX, &Size))
	{
		/* And draw the cursor! */

		TextOut(m_poWindow->m_poDC, Size.cx, (m_iYOffset + (a_iY * m_iHeight)), Cursor, 1);
	}

#endif /* ! __amigaos4__ */

	/* Toggle the highlight state back to normal, remembering that calling SetHighlight() */
	/* above will have set the state of the highlight flag */

	SetHighlight(!(m_iHighlight));
}

/* Written: Sunday 09-May-2010 6:57 pm */

void RFont::DrawText(const char *a_pcText, TInt a_iLength, TInt a_iX, TInt a_iY)
{
	ASSERTM(m_poWindow, "RFont::DrawText() => Window handle not set");

#ifdef __amigaos4__

	TInt NumChars;
	struct TextExtent TextExtent;

	/* Move to the position at which to print, taking into account the left and top border sizes, */
	/* the height of the current font and the baseline of the font, given that IGraphics->Text() */
	/* routine prints at the baseline position, not the top of the font */

	IGraphics->Move(m_poWindow->m_poWindow->RPort, (m_poWindow->m_poWindow->BorderLeft + (a_iX * m_iWidth)),
		(m_poWindow->m_poWindow->BorderTop + (a_iY * m_iHeight) + m_iBaseline));

	/* Calculate the maximum number of characters that can fit in the client area of the window, */
	/* as text is not automatically clipped by the Amiga OS text drawing routine */

	NumChars = IGraphics->TextFit(m_poWindow->m_poWindow->RPort, a_pcText, a_iLength, &TextExtent, NULL, 1,
		m_poWindow->InnerWidth(), m_poWindow->InnerHeight());

	/* And draw as much of the text passed in as will fit in the client area */

	IGraphics->Text(m_poWindow->m_poWindow->RPort, a_pcText, NumChars);

#else /* ! __amigaos4__ */

	// TODO: CAW - Don't use 8
	TextOut(m_poWindow->m_poDC, a_iX * 8, (m_iYOffset + (a_iY * m_iHeight)), a_pcText, a_iLength);

#endif /* ! __amigaos4__ */

}

/* Written: Saturday 21-Aug-2010 8:27 pm */

void RFont::SetHighlight(TBool a_iHighlight)
{
	/* Toggle the text highlight on or off as appropriate */

#ifdef __amigaos4__

	if (a_iHighlight)
	{
		IGraphics->SetAPen(m_poWindow->m_poWindow->RPort, 0);
		IGraphics->SetBPen(m_poWindow->m_poWindow->RPort, 1);
	}
	else
	{
		IGraphics->SetAPen(m_poWindow->m_poWindow->RPort, 1);
		IGraphics->SetBPen(m_poWindow->m_poWindow->RPort, 0);
	}

#else /* ! __amigaos4__ */

	if (a_iHighlight)
	{
		SetBkColor(m_poWindow->m_poDC, m_oText);
		SetTextColor(m_poWindow->m_poDC, m_oBackground);
	}
	else
	{
		SetBkColor(m_poWindow->m_poDC, m_oBackground);
		SetTextColor(m_poWindow->m_poDC, m_oText);
	}

#endif /* ! __amigaos4__ */

	/* And save the highlight state for l8r use */

	m_iHighlight = a_iHighlight;
}

/* Written: Friday 08-Jul-2010 7:42 am, CodeHQ-by-Thames */
/* @param	a_iYOffset	Offset from the top of the window in pixels */
/* This function sets the offset from the top of the window at which text is output */
/* by the RFont class.  This means that when clients call the text drawing functions */
/* and pass in a Y position at which to print, that position is calculated from the */
/* Y offset passed in here, rather than the top of the window.  This allows the */
/* client to treat areas of the window as a sub-windows. */

void RFont::SetYOffset(TInt a_iYOffset)
{
	m_iYOffset = a_iYOffset;
}
