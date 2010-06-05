
#include "StdFuncs.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/intuition.h>
#include <intuition/gui.h>
#include <intuition/imageclass.h>

#else /* ! __amigaos4__ */

/* Written: Saturday 08-May-2010 4:43 pm */

LRESULT CALLBACK WindowProc(HWND a_poWindow, unsigned int a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	LRESULT RetVal;
	CWindow *Window;

	/* Return 0 by default for processed messages */

	RetVal = 0;

	switch (a_uiMessage)
	{
		case WM_DESTROY :
		{
			PostQuitMessage(0);

			break;
		}

		case WM_KEYDOWN :
		{
			/* Get the ptr to the C++ class associated with this window from the window word */
			/* and call the CWindow::OfferKeyEvent() function */

			Window = (CWindow *) GetWindowLong(a_poWindow, GWL_USERDATA);
			Window->OfferKeyEvent(a_oLParam & 0x00ffffff);

			break;
		}

		case WM_PAINT :
		{
			/* Get the ptr to the C++ class associated with this window from the window word */

			Window = (CWindow *) GetWindowLong(a_poWindow, GWL_USERDATA);

			/* Prepare the device context for painting and call the CWindow::Draw() routine. */
			/* If this fails then there isn't much we can do besides ignore the error */

			if ((Window->m_poDC = BeginPaint(a_poWindow, &Window->m_oPaintStruct)) != NULL)
			{
				Window->Draw();
				EndPaint(a_poWindow, &Window->m_oPaintStruct);
			}

			break;
		}

		default :
		{
			RetVal = DefWindowProc(a_poWindow, a_uiMessage, a_oWParam, a_oLParam);

			break;
		}
	}

	return(RetVal);
}

#endif /* ! __amigaos4__ */

/* Written: Monday 08-Feb-2010 7:13 am */

TInt CWindow::Open(const char *a_pccTitle)
{
	TInt RetVal, ScreenWidth, ScreenHeight;

	/* Get the size of the screen so we can open the window filling its full size */

	Utils::GetScreenSize(&ScreenWidth, &ScreenHeight);

#ifdef __amigaos4__

	/* Open the window on the screen, maximised */

	if ((m_poWindow = IIntuition->OpenWindowTags(NULL, WA_Flags, WFLG_SIMPLE_REFRESH,
		WA_Activate, TRUE, WA_Width, ScreenWidth, WA_Height, ScreenHeight, WA_Title, (ULONG) a_pccTitle,
		WA_IDCMP, (IDCMP_CLOSEWINDOW | IDCMP_RAWKEY | IDCMP_REFRESHWINDOW), WA_CloseGadget, TRUE,
		TAG_DONE)) != NULL)
	{
		/* Indicate success */

		RetVal = KErrNone;

		/* Calculate the inner width and height of the window, for l8r use */

		m_iInnerWidth = (m_poWindow->Width - (m_poWindow->BorderRight + m_poWindow->BorderLeft));
		m_iInnerHeight = (m_poWindow->Height - (m_poWindow->BorderBottom + m_poWindow->BorderTop));
	}
	else
	{
		RetVal = KErrGeneral;

		Utils::Info("Unable to open window");
	}

#else /* ! __amigaos4__ */

	HINSTANCE Instance;
	RECT Rect;
	WNDCLASS WndClass;

	/* Assume failure */

	RetVal = KErrGeneral;

	/* Populate a WNDCLASS structure in preparation for registering the window class */

	Instance = GetModuleHandle(NULL);
	WndClass.style = 0;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.cbClsExtra = 10;
	WndClass.cbWndExtra = 20;
	WndClass.hInstance = Instance;
	WndClass.hIcon = 0;
	WndClass.hCursor = LoadCursor (0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = a_pccTitle;

	/* Register the window class and open the window */

	if (RegisterClass(&WndClass))
	{
		if ((m_poWindow = CreateWindow(a_pccTitle, a_pccTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0)) != NULL)
		{
			/* Save a ptr to the window handle for use in the WindowProc() routine */

			SetWindowLong(m_poWindow, GWL_USERDATA, (long) this);

			/* And display the window on the screen, maximised */

			ShowWindow(m_poWindow, SW_MAXIMIZE);

			if (GetClientRect(m_poWindow, &Rect))
			{
				/* Indicate success */

				RetVal = KErrNone;

				m_iInnerWidth = (Rect.right - Rect.left);
				m_iInnerHeight = (Rect.bottom - Rect.top);
			}
			else
			{
				Utils::Info("Unable to obtain window client dimensions");
			}
		}
		else
		{
			Utils::Info("Unable to open window");
		}
	}
	else
	{
		Utils::Info("Unable to register window class");
	}

	/* If anything went wrong, close the window */

	if (RetVal != KErrNone)
	{
		Close();
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Monday 08-Feb-2010 7:18 am */

void CWindow::Close()
{

#ifdef __amigaos4__

	if (m_poWindow)
	{
		IIntuition->CloseWindow(m_poWindow);
		m_poWindow = NULL;
	}

#else /* ! __amigaos4__ */

	if (m_poWindow)
	{
		CloseWindow(m_poWindow);
		m_poWindow = NULL;
	}

#endif /* ! __amigaos4__ */

}

/* Written: Saturday 29-May-2010 1:07 pm*/

void CWindow::DrawNow()
{

#ifdef __amigaos4__

	/* Fill the window background with the standard background colour.  The IIntuition->ShadeRect() */
	/* function is passed the inclusive right and bottom offsets to which to draw, not the size of */
	/* the rect to draw */

	IIntuition->ShadeRect(m_poWindow->RPort, m_poWindow->BorderLeft, m_poWindow->BorderTop,
		(m_poWindow->Width - m_poWindow->BorderRight - 1), (m_poWindow->Height - m_poWindow->BorderBottom - 1),
		LEVEL_NORMAL, BT_BACKGROUND, IDS_NORMAL, IIntuition->GetScreenDrawInfo(m_poWindow->WScreen), TAG_DONE);

	/* And call the derived rendering function to perform a redraw immediately */

	Draw();

#else /* ! __amigaos4__ */

	/* Invalidate the client rect and redraw it, automatically filling the background with the */
	/* background colour first.  If this fails then there isn't much we can do besides ignore */
	/* the error */

	InvalidateRect(m_poWindow, NULL, TRUE);

#endif /* ! __amigaos4__ */

}

#ifdef __amigaos4__

/* Written: Monday 08-Feb-2010 7:33 am */

ULONG CWindow::GetSignal()
{
	return((m_poWindow) ? (1 << m_poWindow->UserPort->mp_SigBit) : 0);
}

/* Written: Saturday 08-May-2010 6:26 pm */

struct Window *CWindow::GetWindow()
{
	return(m_poWindow);
}

#endif /* __amigaos4__ */

/* Written: Monday 08-Feb-2010 7:19 am */

CWindow::~CWindow()
{
	Close();
}
