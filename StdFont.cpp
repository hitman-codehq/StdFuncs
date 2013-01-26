
#include "StdFuncs.h"
#include "StdFont.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/graphics.h>

#elif defined(QT_GUI_LIB)

#include <QtGui/QMainWindow>

#endif /* QT_GUI_LIB */

/* Colours that can be printed by RFont::DrawColouredText().  This must match */
/* STDFONT_NUM_COLOURS in StdFont.h */

static const COLORREF g_aoColours[] = { RGB(0, 0, 0), RGB(163, 21, 21), RGB(0, 128, 0), RGB(0, 0, 255) };

/* Written: Sunday 31-May-2010 1:41 pm */

RFont::RFont(CWindow *a_poWindow)
{
	ASSERTM(a_poWindow, "RFont::RFont() => Window handle must not be NULL");

#ifdef _DEBUG

	m_bHighlight = EFalse;

#endif /* _DEBUG */

	m_bHighlight = EFalse;
	m_iClipWidth = -1; // TODO: CAW - Check for this being -1 in DrawText() + Win32 version and DrawColouredText() ignores this
	m_iWidth = m_iHeight = m_iXOffset = m_iYOffset = 0;
	m_poWindow = a_poWindow;

#ifdef __amigaos4__

	unsigned long Red, Green, Blue;
	TInt Index;
	LONG Pen;

	m_iBaseline = 0;

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow->m_poWindow)
	{
		/* Iterate through the colours and determine the best pen to use for each one. */
		/* This is better done here rather than in RFont::DrawColouredText() every */
		/* time it is called */

		for (Index = 0; Index < STDFONT_NUM_COLOURS; ++Index)
		{
			Red = Utils::Red32(g_aoColours[Index] & 0xff0000);
			Green = Utils::Green32(g_aoColours[Index] & 0xff00);
			Blue = Utils::Blue32(g_aoColours[Index] & 0xff);

			Pen = IGraphics->ObtainBestPen(m_poWindow->m_poWindow->WScreen->ViewPort.ColorMap, Red, Green, Blue, TAG_DONE);
			m_alPens[Index] = Pen;
		}
	}

#elif defined(QT_GUI_LIB)

	m_iBaseline = 0;
	m_poFont = NULL;

#elif defined(WIN32)

	m_poDC = NULL;
	m_poFont = m_poOldFont = NULL;

#endif /* WIN32 */

}

/* Written: Sunday 31-May-2010 3:38 pm */
/* @return	KErrNone if the font was opened successfully, else KErrGeneral */
/* Opens the font for use.  This function can be called at any time but before rendering, you */
/* must first also call RFont::Begin() to setup the rendering context.  Begin() can unfortunately */
/* only be called in response to a system paint event (in other words, from your overridden */
/* CWindow::Draw() function) due to needing to fit in with Qt's architecture.  For cross platform */
/* compatibility, this requirement therefore applies to all platforms, not just Qt. */
/* */
/* After this function has been called, the RFont::Width() and RFont::Height() functions are able */
/* to be called, but no other functions */

TInt RFont::Open()
{
	TInt RetVal;

#ifdef __amigaos4__

	ASSERTM(m_poWindow, "RFont::Open() => Window handle not set");

	/* RFont::Open() cannot fail on the Amiga */

	RetVal = KErrNone;

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow->m_poWindow)
	{
		/* Determine the baseline, width & height of the font from the window */

		m_iBaseline = m_poWindow->m_poWindow->IFont->tf_Baseline;
		m_iWidth = m_poWindow->m_poWindow->IFont->tf_XSize;
		m_iHeight = m_poWindow->m_poWindow->IFont->tf_YSize;
	}

#elif defined(QT_GUI_LIB)

	/* Create a monospace font in the desired point size */

	if ((m_poFont = new QFont("Monospace", 10)) != NULL)
	{
		RetVal = KErrNone;

		/* As this is a programmer's editor, we want a fixed width font */

		m_poFont->setFixedPitch(true);

		/* Determine the width & height of the font */

		QFontMetrics Metrics(*m_poFont);
		m_iBaseline = Metrics.ascent();
		m_iHeight = Metrics.height();
		m_iWidth = Metrics.averageCharWidth();

		/* And assign the font to the window! */

		m_poWindow->m_poWindow->setFont(*m_poFont);

	}
	else
	{
		RetVal = KErrGeneral;
	}

#elif defined(WIN32)

	TInt Height;
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

		// TODO: CAW - This is an expensive routine - can we shorten it?
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

#endif /* WIN32 */

	return(RetVal);
}

/* Written: Monday 05-Jul-2010 7:13 am */
/* Closes the font after use and frees any associated resouces */

void RFont::Close()
{

#ifdef WIN32

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

#else /* ! WIN32 */

	m_iBaseline = 0;

#endif /* WIN32 */

}

/* Written: Saturday 25-Aug-2012 11:30 am */
/* @return	KErrNone if successful, else KErrGeneral if painting could not be started */
/* Prepares the RFont class for writing text using the Draw*() functions.  This should be called */
/* when you are about to render text in response to the overridden CWindow::Draw() function. */
/* This function exists mainly for compatibility with Qt, which introduces some restrictions on */
/* precisely when rendering can occur.  On other operating systems, you can write text using */
/* RFont at any time.  However on Qt you can only write text in response to widget paint events. */
/* So we have to follow Qt's way of working, even though it adds extra complexity to our framework */
/* At the end of rendering, call RFont::End() to release the rendering context */

TInt RFont::Begin()
{
	TInt RetVal;

	/* Assume success */

	RetVal = KErrNone;

#ifdef QT_GUI_LIB

	QWidget *CentralWidget;

	/* Begin the Qt paint process on the window's central widget */

	CentralWidget = m_poWindow->m_poWindow->centralWidget();
	ASSERTM((CentralWidget != NULL), "RFont::Begin() => Central widget has not been assigned to window");

	if (m_oPainter.begin(CentralWidget))
	{
		/* Save the background and text colours for l8r use */

		m_oBackground = m_oPainter.background().color();
		m_oText = m_oPainter.pen().color();
	}
	else
	{
		RetVal = KErrGeneral;

		Utils::Info("RFont::Begin() => Unable to begin painting to device");
	}

#endif /* QT_GUI_LIB */

#ifdef _DEBUG

	if (RetVal == KErrNone)
	{
		m_bBeginCalled = ETrue;
	}

#endif /* _DEBUG */

	return(RetVal);
}

/* Written: Saturday 25-Aug-2012 11:32 am */
/* Ends the rendering process started by RFont::Begin().  All calls to RFont::Begin() must */
/* have a matching call to RFont::End() */

void RFont::End()
{
	ASSERTM(m_bBeginCalled, "RFont::End() => RFont::Begin() must be called before RFont::End()");

#ifdef _DEBUG

	m_bBeginCalled = EFalse;

#endif /* _DEBUG */

	/* Set the text and background colours back to their default as these are held by the */
	/* operating system and will persist across instances of the RFont class */

	SetHighlight(EFalse);

#ifdef QT_GUI_LIB

	/* Finish the Qt paint process */

	m_oPainter.end();

#endif /* QT_GUI_LIB */

}

/* Written: Tuesday 08-Jun-2010 6:22 am */

void RFont::DrawCursor(const char *a_pccText, TInt a_iX, TInt a_iY, TBool a_iDrawCharacter)
{
	ASSERTM(m_poWindow, "RFont::DrawCursor() => Window handle not set");
	ASSERTM(m_bBeginCalled, "RFont::DrawCursor() => RFont::Begin() must be called before RFont::DrawCursor()");

	char Space[1] = { ' ' };
	const char *Cursor;

	/* Invert the current highlight state before drawing the cursor and draw the letter under */
	/* the cursor.  This will cause it to be highlighted.  We toggle the highlight rather than */
	/* use ETrue and EFalse so that the cursor works within highlighted blocks of text as well */

	SetHighlight(!(m_bHighlight));

	/* Draw the text or a space instead as requested */

	Cursor = (a_iDrawCharacter) ? &a_pccText[a_iX] : Space;

#ifdef __amigaos4__

	TInt X;

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow->m_poWindow)
	{
		/* Calculate the position at which to draw the cursor */

		X = (m_poWindow->m_poWindow->BorderLeft + m_iXOffset + (a_iX * m_iWidth));

		/* Now ensure that the cursor is within the bounds of the clipping area before drawing it */

		if ((X + m_iWidth) <= (m_poWindow->m_poWindow->BorderLeft + m_iXOffset + m_iClipWidth))
		{
			/* Move to the position at which to print, taking into account the left and top border sizes, */
			/* the height of the current font and the baseline of the font, given that IGraphics->Text() */
			/* routine prints at the baseline position, not the top of the font */

			IGraphics->Move(m_poWindow->m_poWindow->RPort, X, (m_poWindow->m_poWindow->BorderTop + m_iYOffset + (a_iY * m_iHeight) + m_iBaseline));

			/* And draw the cursor! */

			IGraphics->Text(m_poWindow->m_poWindow->RPort, Cursor, 1);
		}
	}

#elif defined(QT_GUI_LIB)

	TInt X, Y;

	/* Calculate the position at which to draw the cursor */

	X = (m_iXOffset + (a_iX * m_iWidth));
	Y = (m_iYOffset + (a_iY * m_iHeight));

	/* Printing inverted text on Linux doesn't seem to work, so fill the cursor rectangle using the */
	/* background colour */

	m_oPainter.fillRect(X, Y, m_iWidth, m_iHeight, m_oText);

	/* And render the inverted character in the cursor, taking into account that QPainter::drawText() */
	/* uses the Y position as the baseline of the font, not as the top */

	QByteArray String(Cursor, 1);
	m_oPainter.drawText(X, (Y + m_iBaseline), String);

#elif defined(WIN32)

	SIZE Size;

	/* Given the X position of the cursor, determine the number of pixels between the left of */
	/* the window and the position at which the cursor is to be displayed, and use this and the */
	/* font height to display the cursor in the correct position */

	if (GetTextExtentPoint32(m_poWindow->m_poDC, a_pccText, a_iX, &Size))
	{
		/* And draw the cursor! */

		TextOut(m_poWindow->m_poDC, (m_iXOffset + Size.cx), (m_iYOffset + (a_iY * m_iHeight)), Cursor, 1);
	}

#endif /* WIN32 */

	/* Toggle the highlight state back to normal, remembering that calling SetHighlight() */
	/* above will have set the state of the highlight flag */

	SetHighlight(!(m_bHighlight));
}

/* Written: Sunday 09-May-2010 6:57 pm */
/* @param	a_pccText	Ptr to string to be drawn to the screen */
/*			a_iLength	Length of the string pointed to by a_pccText */
/*			a_iX		X position in the window at which to draw */
/*			a_iY		Y position in the window at which to draw */
/* Draws a string to the window the font is assigned to at the specified X and Y */
/* positions */

void RFont::DrawText(const char *a_pccText, TInt a_iLength, TInt a_iX, TInt a_iY)
{
	ASSERTM(a_pccText, "RFont::DrawText() => Text ptr must not be NULL");
	ASSERTM(m_poWindow, "RFont::DrawText() => Window handle not set");
	ASSERTM(m_bBeginCalled, "RFont::DrawText() => RFont::Begin() must be called before RFont::DrawText()");

#ifdef __amigaos4__

	TInt NumChars, Width;
	struct TextExtent TextExtent;

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow->m_poWindow)
	{
		/* Move to the position at which to print, taking into account the left and top border sizes, */
		/* the height of the current font and the baseline of the font, given that IGraphics->Text() */
		/* routine prints at the baseline position, not the top of the font */

		IGraphics->Move(m_poWindow->m_poWindow->RPort, (m_poWindow->m_poWindow->BorderLeft + m_iXOffset + (a_iX * m_iWidth)),
			(m_poWindow->m_poWindow->BorderTop + m_iYOffset + (a_iY * m_iHeight) + m_iBaseline));

		/* Calculate the maximum number of characters that can fit in the client area of the window, */
		/* as text is not automatically clipped by the Amiga OS text drawing routine.  The text may */
		/* not start at the very left of the screen so take this into account */

		Width = (m_iClipWidth - (a_iX * m_iWidth));
		Width = MAX(0, Width);

		NumChars = IGraphics->TextFit(m_poWindow->m_poWindow->RPort, a_pccText, a_iLength, &TextExtent, NULL, 1,
			Width, m_poWindow->InnerHeight());

		/* And draw as much of the text passed in as will fit in the client area */

		IGraphics->Text(m_poWindow->m_poWindow->RPort, a_pccText, NumChars);
	}

#elif defined(QT_GUI_LIB)

	/* Render the string passed in, taking into account that QPainter::drawText() uses the Y position as */
	/* the baseline of the font, not as the top */

	QByteArray String(a_pccText, a_iLength);
	m_oPainter.drawText((m_iXOffset + (a_iX * m_iWidth)), (m_iYOffset + (a_iY * m_iHeight) + m_iBaseline), String);

#elif defined(WIN32)

	TextOut(m_poWindow->m_poDC, (m_iXOffset + (a_iX * m_iWidth)), (m_iYOffset + (a_iY * m_iHeight)), a_pccText, a_iLength);

#endif /* WIN32 */

}

/* Written: Thursday 01-Dec-2011 8:43 pm, Munich Airport awaiting flight EK 052 to Dubai */
/* @param	a_pccText	Ptr to string to be drawn to the screen */
/*			a_iX		X position in the window at which to draw */
/*			a_iY		Y position in the window at which to draw */
/* Draws a string to the window the font is assigned to at the specified X and Y */
/* positions.  Unline RFont::DrawText(), this text contains embedded codes that */
/* specify the colour to use for each run of characters.  The encoded text is of the */
/* following format: */
/* */
/* Length Colour Text[Length] 0 */
/* */
/* The Length byte specifies the length of the upcoming run of text.  It is 0 to terminate */
/* the string.  The Colour byte specifies the colour in which to draw and following this are */
/* Length bytes of the text itself.  After this, the sequence repeats or is terminated with 0 */

void RFont::DrawColouredText(const char *a_pccText, TInt a_iX, TInt a_iY)
{
	TInt Colour, Length;

	ASSERTM(a_pccText, "RFont::DrawColouredText() => Text ptr must not be NULL");
	ASSERTM(m_poWindow, "RFont::DrawColouredText() => Window handle not set");
	ASSERTM(m_bBeginCalled, "RFont::DrawColouredText() => RFont::Begin() must be called before RFont::DrawColouredText()");

#ifdef __amigaos4__

	TInt NumChars, Width;
	struct TextExtent TextExtent;

	/* Determine how much space we have in which to print.  The text may not start at the very left */
	/* of the screen so take this into account */

	Width = (m_iClipWidth - (a_iX * m_iWidth));
	Width = MAX(0, Width);

	/* Iterate through the source text and display the runs of characters in the required colour */

	while (*a_pccText)
	{
		/* Get the length of the run and the colour to display the run in */

		Length = *a_pccText++;
		Colour = *a_pccText++;
		ASSERTM((Colour < STDFONT_NUM_COLOURS), "RFont::DrawColouredText() => Colour index out of range");

		/* Move to the position at which to print, taking into account the left and top border sizes, */
		/* the height of the current font and the baseline of the font, given that IGraphics->Text() */
		/* routine prints at the baseline position, not the top of the font */

		IGraphics->Move(m_poWindow->m_poWindow->RPort, (m_poWindow->m_poWindow->BorderLeft + m_iXOffset + (a_iX * m_iWidth)),
			(m_poWindow->m_poWindow->BorderTop + m_iYOffset + (a_iY * m_iHeight) + m_iBaseline));

		/* Calculate the maximum number of characters that can fit in the client area of the window, */
		/* as text is not automatically clipped by the Amiga OS text drawing routine.  Note that the */
		/* clip width reduces as we print the parts of the string */

		NumChars = IGraphics->TextFit(m_poWindow->m_poWindow->RPort, a_pccText, Length, &TextExtent, NULL, 1,
			Width, m_poWindow->InnerHeight());

		/* If nothing fits then we may as well break out of the loop */

		if (NumChars == 0)
		{
			break;
		}

		/* Display the text in the required colour.  If the highlight is set then */
		/* the colours will already have been inversed so leave them as they are */

		if (!(m_bHighlight))
		{
			IGraphics->SetAPen(m_poWindow->m_poWindow->RPort, m_alPens[Colour]);
		}

		IGraphics->Text(m_poWindow->m_poWindow->RPort, a_pccText, NumChars);

		/* And prepare for the next run to be displayed */

		a_iX += Length;
		a_pccText += Length;
		Width -= TextExtent.te_Width;

		ASSERTM((Width >= 0), "RFont::DrawColouredText() => Out of space");
	}

#elif defined(QT_GUI_LIB)

	/* Iterate through the source text and display the runs of characters in the required colour */

	while (*a_pccText)
	{
		/* Get the length of the run and the colour to display the run in */

		Length = *a_pccText++;
		Colour = *a_pccText++;
		ASSERTM((Colour < STDFONT_NUM_COLOURS), "RFont::DrawColouredText() => Colour index out of range");

		/* Display the text in the required colour, taking into account that QPainter::drawText() uses */
		/* the Y position as the baseline of the font, not as the top.  If the highlight is set then */
		/* the colours will already have been inversed so leave them as they are */

		if (!(m_bHighlight))
		{
			QPen Pen(QColor((g_aoColours[Colour] >> 26), ((g_aoColours[Colour] & 0xff00) >> 8), (g_aoColours[Colour] & 0xff)));
			m_oPainter.setPen(Pen);
		}

		QByteArray String(a_pccText, Length);

		m_oPainter.drawText((m_iXOffset + (a_iX * m_iWidth)), (m_iYOffset + (a_iY * m_iHeight) + m_iBaseline), String);

		/* And prepare for the next run to be displayed */

		a_iX += Length;
		a_pccText += Length;
	}

#else /* ! QT_GUI_LIB */

	/* Iterate through the source text and display the runs of characters in the required colour */

	while (*a_pccText)
	{
		/* Get the length of the run and the colour to display the run in */

		Length = *a_pccText++;
		Colour = *a_pccText++;
		ASSERTM((Colour < STDFONT_NUM_COLOURS), "RFont::DrawColouredText() => Colour index out of range");

		/* Display the text in the required colour.  If the highlight is set then */
		/* the colours will already have been inversed so leave them as they are */

		if (!(m_bHighlight))
		{
			SetTextColor(m_poWindow->m_poDC, g_aoColours[Colour]);
		}

		TextOut(m_poWindow->m_poDC, (m_iXOffset + (a_iX * m_iWidth)), (m_iYOffset + (a_iY * m_iHeight)), a_pccText, Length);

		/* And prepare for the next run to be displayed */

		a_iX += Length;
		a_pccText += Length;
	}

#endif /* ! QT_GUI_LIB */

}

/* Written: Saturday 21-Aug-2010 8:27 pm */

void RFont::SetHighlight(TBool a_bHighlight)
{
	ASSERTM(m_poWindow, "RFont::SetHighlight() => Window handle not set");

	/* Toggle the text highlight on or off as appropriate */

#ifdef __amigaos4__

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow->m_poWindow)
	{
		if (a_bHighlight)
		{
			IGraphics->SetAPen(m_poWindow->m_poWindow->RPort, 0);
			IGraphics->SetBPen(m_poWindow->m_poWindow->RPort, 1);
		}
		else
		{
			IGraphics->SetAPen(m_poWindow->m_poWindow->RPort, 1);
			IGraphics->SetBPen(m_poWindow->m_poWindow->RPort, 0);
		}
	}

#elif defined(QT_GUI_LIB)

	if (a_bHighlight)
	{
		m_oPainter.setBrush(m_oText);
		m_oPainter.setPen(m_oBackground);
	}
	else
	{
		m_oPainter.setBrush(m_oBackground);
		m_oPainter.setPen(m_oText);
	}

#elif defined(WIN32)

	if (a_bHighlight)
	{
		SetBkColor(m_poWindow->m_poDC, m_oText);
		SetTextColor(m_poWindow->m_poDC, m_oBackground);
	}
	else
	{
		SetBkColor(m_poWindow->m_poDC, m_oBackground);
		SetTextColor(m_poWindow->m_poDC, m_oText);
	}

#endif /* WIN32 */

	/* And save the highlight state for l8r use */

	m_bHighlight = a_bHighlight;
}

/* Written: Wednesday 30-Nov-2011 5:49 am, Söflingen */
/* @param	a_iClipWidth    Number of pixels to draw horizontally before clipping */
/* Sets the clipping width of the area into which the font will draw.  This is the */
/* number of pixels starting at the value passed into RFont::SetXOffset() - it is */
/* an offset, not an absolute value! */

void RFont::SetClipWidth(TInt a_iClipWidth)
{
	m_iClipWidth = a_iClipWidth;
}

/* Written: Thursday 31-May-2012 7:20 am, CodeHQ Ehinger Tor */
/* @param	a_iXOffset	Offset from the left of the window in pixels */
/* This function sets the offset from the left of the window at which text is output */
/* by the RFont class.  This means that when clients call the text drawing functions */
/* and pass in an X position at which to print, that position is calculated from the */
/* X offset passed in here, rather than the left of the window.  This allows the */
/* client to treat areas of the window as a sub-windows. */

void RFont::SetXOffset(TInt a_iXOffset)
{
	m_iXOffset = a_iXOffset;
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
