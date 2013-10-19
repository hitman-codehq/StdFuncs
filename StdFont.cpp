
#include "StdFuncs.h"
#include "StdFont.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/diskfont.h>
#include <proto/graphics.h>

#elif defined(QT_GUI_LIB)

#include "Qt/StdWindow.h"

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
	m_iClipWidth = m_iClipHeight = -1; // TODO: CAW - Check for this being -1 in DrawText()
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
			Red = Utils::Red32(g_aoColours[Index] & 0xff);
			Green = Utils::Green32(g_aoColours[Index] & 0xff00);
			Blue = Utils::Blue32(g_aoColours[Index] & 0xff0000);

			Pen = IGraphics->ObtainBestPen(m_poWindow->m_poWindow->WScreen->ViewPort.ColorMap, Red, Green, Blue, TAG_DONE);
			m_alPens[Index] = Pen;
		}
	}

	/* We are using the default font by default */

	m_poFont = m_poOldFont = NULL;

#elif defined(QT_GUI_LIB)

	m_iBaseline = 0;
	m_poFont = NULL;

#elif defined(WIN32)

	m_poDC = NULL;
	m_poFont = m_poOldFont = NULL;

#endif /* WIN32 */

}

/**
 * Opens the default or a user specified font for use.
 * Opens the font for use.  This function can be called at any time but before rendering, you
 * must first also call RFont::Begin() to setup the rendering context.  Begin() can unfortunately
 * only be called in response to a system paint event (in other words, from your overridden
 * CWindow::Draw() function) due to needing to fit in with Qt's architecture.  For cross platform
 * compatibility, this requirement therefore applies to all platforms, not just Qt.
 *
 * After this function has been called, the RFont::Width() and RFont::Height() functions are able
 * to be called, but no other functions.
 *
 * @date	Sunday 31-May-2010 3:38 pm
 * @param	a_pccFont	Ptr to the name of the font to be opened, which is platform specific.  If NULL
 *						then a platform specific generic courier style font will be selected
 * @return	KErrNone if the font was opened successfully, else KErrGeneral
 */

TInt RFont::Open(const char *a_pccFont)
{
	TInt RetVal;

#ifdef __amigaos4__

	struct TextAttr TextAttr = { a_pccFont, 0, 0, 0 };

	ASSERTM(m_poWindow, "RFont::Open() => Window handle not set");

	/* RFont::Open() cannot fail on the Amiga */

	RetVal = KErrNone;

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow->m_poWindow)
	{
		/* If a specific font has been specified then try to load it from disk and make it the rastport's */
		/* default font */

		if (a_pccFont)
		{
			/* Use the same size font as the currently selected one */

			TextAttr.ta_YSize = m_poWindow->m_poWindow->RPort->Font->tf_YSize;

			/* And try to open the font */

			if ((m_poFont = IDiskfont->OpenDiskFont(&TextAttr)) != NULL)
			{
				m_poOldFont = m_poWindow->m_poWindow->RPort->Font;
				IGraphics->SetFont(m_poWindow->m_poWindow->RPort, m_poFont);
			}
			else
			{
				Utils::Info("RFont::Open() => Unable to open font \"%s\"", a_pccFont);
			}
		}

		/* Determine the baseline, width & height of the font from the window */

		m_iBaseline = m_poWindow->m_poWindow->RPort->Font->tf_Baseline;
		m_iWidth = m_poWindow->m_poWindow->RPort->Font->tf_XSize;
		m_iHeight = m_poWindow->m_poWindow->RPort->Font->tf_YSize;
	}

#elif defined(QT_GUI_LIB)

	/* If no font has been specified then use "Monospace" */

	if (!(a_pccFont))
	{
		a_pccFont = "Monospace";
	}

	/* Create a monospace font in the desired point size */

	if ((m_poFont = new QFont(a_pccFont, 10)) != NULL)
	{
		RetVal = KErrNone;

		/* As this is a programmer's editor, we want a fixed width font */

		m_poFont->setFixedPitch(true);

		/* Determine the width & height of the font */

		QFontMetrics Metrics(*m_poFont);
		m_iBaseline = Metrics.ascent();
		m_iHeight = Metrics.height();
		m_iWidth = Metrics.averageCharWidth();

		/* And assign the font to the window */

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
		/* If no font has been specified then use "Courier" which is as ugly as sin but has been */
		/* the default on Brunel since day one and fits lots of lines on my netbook! */

		if (!(a_pccFont))
		{
			a_pccFont = "Courier";
		}

		/* Convert the font size from the desired point size to pixels per inch and attempt to open a font */
		/* that uses that size */

		Height = -MulDiv(10, GetDeviceCaps(m_poWindow->m_poDC, LOGPIXELSY), 72);

		// TODO: CAW - This is an expensive routine - can we shorten it?
		if ((m_poFont = CreateFont(Height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, (FF_MODERN | FIXED_PITCH), a_pccFont)) != NULL)
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

#ifdef __amigaos4__

	/* If a user defined font was loaded and assigned to the rastport, set the rastport's font back */
	/* to the default and free the font */

	if (m_poFont)
	{
		if (m_poOldFont)
		{
			IGraphics->SetFont(m_poWindow->m_poWindow->RPort, m_poOldFont);
			m_poOldFont = NULL;
		}

		IGraphics->CloseFont(m_poFont);
		m_poFont = NULL;
	}

#elif defined(WIN32) && !defined(QT_GUI_LIB)

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

#else /* ! defined(WIN32) && !defined(QT_GUI_LIB) */

	m_iBaseline = 0;

#endif /* ! defined(WIN32) && !defined(QT_GUI_LIB) */

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

	/* Set the background mode to opaque, as we want to forcibly draw the cursor over the top of the background */

	m_oPainter.setBackgroundMode(Qt::OpaqueMode);

	/* And render the inverted character in the cursor, taking into account that QPainter::drawText() */
	/* uses the Y position as the baseline of the font, not as the top */

	QByteArray String(Cursor, 1);
	m_oPainter.drawText(X, (Y + m_iBaseline), String);

	/* Set the background mode back to transparent so that if there is a pretty transparent background or a */
	/* gradient in the background it is displayed correctly */

	m_oPainter.setBackgroundMode(Qt::TransparentMode);

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

/* Written: Thursday 01-Dec-2011 8:43 pm, Munich Airport, awaiting flight EK 052 to Dubai */
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

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow->m_poWindow)
	{
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
	}

#elif defined(QT_GUI_LIB)

	/* Iterate through the source text and display the runs of characters in the required colour */

	while (*a_pccText)
	{
		/* Get the length of the run and the colour to display the run in */

		Length = *a_pccText++;
		Colour = *a_pccText++;
		ASSERTM((Colour < STDFONT_NUM_COLOURS), "RFont::DrawColouredText() => Colour index out of range");

		/* If the highlight is set then the colours will already have been inversed so don't change them */

		if (!(m_bHighlight))
		{
			QPen Pen(QColor((g_aoColours[Colour] & 0xff), ((g_aoColours[Colour] & 0xff00) >> 8), (g_aoColours[Colour] >> 16)));
			m_oPainter.setPen(Pen);
		}

		/* Render the string passed in, taking into account that QPainter::drawText() uses the Y position as */
		/* the baseline of the font, not as the top */

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
		m_oPainter.setBackground(m_oText);
		m_oPainter.setPen(m_oBackground);

		/* Set the background mode to opaque, as we want to forcibly draw the highlight over the top of the background */

		m_oPainter.setBackgroundMode(Qt::OpaqueMode);
	}
	else
	{
		m_oPainter.setBackground(m_oBackground);
		m_oPainter.setPen(m_oText);

		/* Set the background mode back to transparent so that if there is a pretty transparent background or a */
		/* gradient in the background it is displayed correctly */

		m_oPainter.setBackgroundMode(Qt::TransparentMode);
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

/**
 * Defines the rectangular area used for drawing text.
 * By default the RFont class will use the entire client area of the window in which it
 * used.  This function can be used to define a sub area of that window into which to
 * draw.  It acts as both a position setting and a clip setting function.  The text will
 * be drawn starting at offset a_iXOffset, a_YOffset from the inside border of the parent
 * window and will be clipped after a_iWidth pixels horizontally and a_iHeight pixels
 * vertically.
 *
 * @pre	This function must be called after RFont::Begin() for clipping to take effect on
 * all platforms.
 *
 * @date	Friday 12-Jul-2013 6:33 am, Code HQ Ehinger Tor
 * @param	a_iXOffset	X position in pixels at which to begin drawing
 * @param	a_iYOffset	Y position in pixels at which to begin drawing
 * @param	a_iWidth	# of pixels to draw horizontally before clipping
 * @param	a_iHeight	# of pixels to draw vertically before clipping
 */

void RFont::SetDrawingRect(TInt a_iXOffset, TInt a_iYOffset, TInt a_iWidth, TInt a_iHeight)
{
	ASSERTM((a_iXOffset >= 0), "RFont::SetDrawingRect() => X offset must be >= 0");
	ASSERTM((a_iYOffset >= 0), "RFont::SetDrawingRect() => Y offset must be >= 0");
	ASSERTM((a_iWidth >= 0), "RFont::SetDrawingRect() => Width must be >= 0");
	ASSERTM((a_iHeight >= 0), "RFont::SetDrawingRect() => Height must be >= 0");
	ASSERTM((m_bBeginCalled != EFalse), "RFont::SetDrawingRect() => RFont::Begin() must be called first");

	/* Save the coordinates of the clipping rectangle */

	m_iXOffset = a_iXOffset;
	m_iYOffset = a_iYOffset;
	m_iClipWidth = a_iWidth;
	m_iClipHeight = a_iHeight;

#ifdef QT_GUI_LIB

	/* And set the clipping rectangle in the QPainter, so that it is active for all operations */

	m_oPainter.setClipRect(m_iXOffset, m_iYOffset, m_iClipWidth, m_iClipHeight);

#endif /* QT_GUI_LIB */

}
