
#include "StdFuncs.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/intuition.h>

#else /* ! __amigaos4__ */

/* Written: Saturday 08-May-2010 4:43 pm */

// TODO: CAW - WindowFunc?  Change name of a_hWindow to match others?
LRESULT CALLBACK WindowProcedure(HWND a_hWindow, unsigned int a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	LRESULT RetVal;
	CWindow *Window;

	RetVal = 0;

	switch (a_uiMessage)
	{
		case WM_DESTROY :
		{
			PostQuitMessage(0);

			break;
		}

		case WM_PAINT :
		{
			Window = (CWindow *) GetWindowLong(a_hWindow, GWL_USERDATA);

			// TODO: CAW - Bodgey
			Window->m_poDC = BeginPaint(a_hWindow, &Window->m_oPaintStruct);

			Window->Draw();

			EndPaint(a_hWindow, &Window->m_oPaintStruct);

			break;
		}

		default :
		{
			RetVal = DefWindowProc(a_hWindow, a_uiMessage, a_oWParam, a_oLParam);

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

	if ((m_poWindow = IIntuition->OpenWindowTags(NULL, WA_IDCMP, IDCMP_CLOSEWINDOW, WA_CloseGadget, TRUE,
		WA_Activate, TRUE, WA_Width, ScreenWidth, WA_Height, ScreenHeight, WA_Title, (ULONG) a_pccTitle,
		TAG_DONE)) != NULL)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = KErrGeneral;

		Utils::Info("Unable to open window");
	}

#else /* ! __amigaos4__ */

	HINSTANCE Instance;
	WNDCLASS WndClass;

	/* Assume failure */

	RetVal = KErrGeneral;

	/* Populate a WNDCLASS structure in preparation for registering the window class */
	Instance = GetModuleHandle(NULL);
	WndClass.style = 0;
	WndClass.lpfnWndProc = WindowProcedure;
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
			/* Indicate success */

			RetVal = KErrNone;

			// TODO: CAW
			SetWindowLong(m_poWindow, GWL_USERDATA, (long) this);

			/* And display the window on the screen, maximised */

			ShowWindow(m_poWindow, SW_MAXIMIZE);
			UpdateWindow(m_poWindow);
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

#ifdef __amigaos4__

/* Written: Monday 08-Feb-2010 7:33 am */

ULONG CWindow::GetSignal()
{
	return((m_poWindow) ? (1 << m_poWindow->UserPort->mp_SigBit) : 0);
}

#endif /* __amigaos4__ */

/* Written: Monday 08-Feb-2010 7:19 am */

CWindow::~CWindow()
{
	Close();
}
