
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdGadgets.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <proto/intuition.h>
#include <proto/utility.h>
#include <reaction/reaction.h>
#include <intuition/gui.h>
#include <intuition/imageclass.h>

#else /* ! __amigaos4__ */

/* Array of key mappings for mapping Windows keys onto standard keys */

static const SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, VK_SHIFT }, { STD_KEY_CONTROL, VK_CONTROL }, { STD_KEY_BACKSPACE, VK_BACK },
	{ STD_KEY_ENTER, VK_RETURN }, { STD_KEY_UP, VK_UP }, { STD_KEY_DOWN, VK_DOWN },
	{ STD_KEY_LEFT, VK_LEFT }, { STD_KEY_RIGHT, VK_RIGHT }, { STD_KEY_HOME, VK_HOME },
	{ STD_KEY_END, VK_END }, { STD_KEY_PGUP, VK_PRIOR }, { STD_KEY_PGDN, VK_NEXT },
	{ STD_KEY_DELETE, VK_DELETE }, { STD_KEY_TAB, VK_TAB }, { STD_KEY_F3, VK_F3 }
};

#define NUM_KEYMAPPINGS 15

#endif /* ! __amigaos4__ */

CWindow *CWindow::m_poRootWindow; /* Ptr to root window on which all other windows open */

#ifdef __amigaos4__

/* Written: Saturday 20-Nov-2010 11:05 am */

void CWindow::IDCMPFunction(struct Hook *a_poHook, Object * /*a_poObject*/, struct IntuiMessage *a_poIntuiMessage)
{
	struct TagItem *TagItem;
	CStdGadget *Gadget;
	CWindow *Window;

	/* If this is a message from a BOOPSI object saying that it has been updated, find the object and */
	/* map it onto its matching gadget and call the gadget's Updated() function */

	if (a_poIntuiMessage->Class == IDCMP_IDCMPUPDATE)
	{
		/* Get the gadget's unique ID */

		if ((TagItem = IUtility->FindTagItem(GA_ID, (struct TagItem *) a_poIntuiMessage->IAddress)) != NULL)
		{
			Window = (CWindow *) a_poHook->h_Data;

			/* Iterate through the window's list of gadgets and search for the one matching the */
			/* BOOPSI gadget's ID */

			if ((Gadget = Window->m_oGadgets.GetHead()) != NULL)
			{
				do
				{
					if (TagItem->ti_Data == (ULONG) Gadget->GadgetID())
					{
						/* Got it!  Call the gadget's Updated() routine so that it can notify the */
						/* client of the update */

						Gadget->Updated();

						break;
					}
				}
				while ((Gadget = Window->m_oGadgets.GetSucc(Gadget)) != NULL);
			}
		}
	}
}

#else /* ! __amigaos4__ */

/* Written: Saturday 08-May-2010 4:43 pm */

LRESULT CALLBACK CWindow::WindowProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	int Index;
	LRESULT RetVal;
	CStdGadget *Gadget;
	CWindow *Window;

	/* Return 0 by default for processed messages */

	RetVal = 0;

	/* Get the ptr to the C++ class associated with this window from the window word */

	Window = (CWindow *) GetWindowLong(a_poWindow, GWL_USERDATA);

	switch (a_uiMessage)
	{
		case WM_DESTROY :
		{
			/* When the WM_DESTROY message is received, the Windows window has already been */
			/* destroyed so set its handle to NULL so that we do not try to destroy it again */
			/* in CWindow's destructor */

			Window->m_poWindow = NULL;
			PostQuitMessage(0);

			break;
		}

		case WM_COMMAND :
		{
			/* Call the CWindow::HandleCommand() function so the client can process the message */

			Window->HandleCommand(LOWORD(a_oWParam));

			break;
		}

		case WM_CHAR :
		{
			/* Call the CWindow::OfferKeyEvent() function, passing in only valid ASCII characters */

			if ((a_oWParam >= ' ') && (a_oWParam <= '~'))
			{
				Window->OfferKeyEvent(a_oWParam, ETrue);
			}

			break;
		}

		case WM_KEYDOWN :
		case WM_KEYUP :
		{
			/* Scan through the key mappings and find the one that has just been pressed */

			for (Index = 0; Index < NUM_KEYMAPPINGS; ++Index)
			{
				if (g_aoKeyMap[Index].m_iNativeKey == (int) a_oWParam)
				{
					break;
				}
			}

			/* If it was a known key then convert it to the standard value and pass it to the */
			/* CWindow::OfferKeyEvent() function */

			if (Index < NUM_KEYMAPPINGS)
			{
				Window->OfferKeyEvent(g_aoKeyMap[Index].m_iStdKey, (a_uiMessage == WM_KEYDOWN));
			}

			break;
		}

		case WM_LBUTTONDOWN :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam));

			break;
		}

		case WM_PAINT :
		{
			/* Prepare the device context for painting and call the CWindow::Draw() routine. */
			/* If this fails then there isn't much we can do besides ignore the error */

			if ((Window->m_poDC = BeginPaint(a_poWindow, &Window->m_oPaintStruct)) != NULL)
			{
				Window->Draw();
				EndPaint(a_poWindow, &Window->m_oPaintStruct);
			}

			break;
		}

		case WM_VSCROLL :
		{
			/* Iterate through the window's list of gadgets and search for the one matching the */
			/* HWND of the slider just moved */

			if ((Gadget = Window->m_oGadgets.GetHead()) != NULL)
			{
				do
				{
					if (a_oLParam == (LPARAM) Gadget->m_poGadget)
					{
						/* Got it!  Call the gadget's Updated() routine so that it can notify the */
						/* client of the update, letting it know what type of update this is */

						Gadget->Updated(LOWORD(a_oWParam));

						break;
					}
				}
				while ((Gadget = Window->m_oGadgets.GetSucc(Gadget)) != NULL);
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

/* Written: Wednesday 13-Oct-2010 7:29 am */

void CWindow::Activate()
{
	ASSERTM(m_poWindow, "CWindow::Activate() => Window must be already open");

#ifdef __amigaos4__

	IIntuition->SetAttrs(m_poWindowObj, WINDOW_FrontBack, WT_FRONT, TAG_DONE);
	IIntuition->SetWindowAttrs(m_poWindow, WA_Activate, TRUE, sizeof(BOOL));

#else /* ! __amigaos4__ */

#ifdef _DEBUG

	// TODO: CAW - Need a function to do this + what about checking for Amiga OS version above?
	ASSERTM((SetActiveWindow(m_poWindow) != NULL), "CWindow::Activate() => Unable to activate window");

#else /* ! _DEBUG */

	SetActiveWindow(m_poWindow);

#endif /* ! _DEBUG */

#endif /* ! __amigaos4__ */

}

/* Written: Sunday 21-Nov-2010 8:11 am */

void CWindow::Attach(CStdGadget *a_poGadget)
{
	ASSERTM((a_poGadget != NULL), "CWindow::Attach() => No gadget to be attached passed in");
	ASSERTM((m_poWindow != NULL), "CWindow::Attach() => Window not yet open");

	// TODO: CAW - HACK!
	m_iInnerWidth -= 20;

#ifdef __amigaos4__

	IIntuition->AddGList(m_poWindow, (struct Gadget *) a_poGadget->m_poGadget, -1, -1, NULL);
	IIntuition->RefreshGList((struct Gadget *) a_poGadget->m_poGadget, m_poWindow, NULL, -1);

#endif /* __amigaos4__ */

	m_oGadgets.AddTail(a_poGadget);
}

/* Written: Monday 08-Feb-2010 7:13 am */

TInt CWindow::Open(const char *a_pccTitle, const char *a_pccPubScreenName)
{
	TInt RetVal, ScreenWidth, ScreenHeight;

	/* Get the size of the screen so we can open the window filling its full size */

	Utils::GetScreenSize(&ScreenWidth, &ScreenHeight);

#ifdef __amigaos4__

	/* Assume failure */

	RetVal = KErrGeneral;

	/* Setup an IDCMP hook that can be used for monitoring gadgets for extra information not */
	/* provided by Reaction, such as the movement of proportional gadgets */

	m_oIDCMPHook.h_Entry = (ULONG (*)()) IDCMPFunction;
	m_oIDCMPHook.h_Data = this;

	/* Create a Reaction Window and open it on the requested screen at the maximum size of */
	/* the screen.  If no screen name is specified, fall back to the Workbench */

	m_poWindowObj = (Object *) WindowObject,
		WA_Title, (ULONG) a_pccTitle, WINDOW_Position, WPOS_CENTERSCREEN,
		WA_PubScreenName, a_pccPubScreenName, WA_PubScreenFallBack, TRUE,
		WA_Width, ScreenWidth, WA_Height, ScreenHeight, WA_Activate, TRUE,
		WA_CloseGadget, TRUE, WA_DepthGadget, TRUE, WA_DragBar, TRUE,
		WINDOW_IDCMPHook, &m_oIDCMPHook, WINDOW_IDCMPHookBits, IDCMP_IDCMPUPDATE,
		WA_IDCMP, (IDCMP_CLOSEWINDOW | IDCMP_IDCMPUPDATE | IDCMP_MENUPICK | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY | IDCMP_REFRESHWINDOW),
	EndWindow;

	if (m_poWindowObj)
	{
		if (RA_OpenWindow(m_poWindowObj))
		{
			/* Get a ptr to the underlying Intuition Window, as it is handy to have for such */
			/* things as obtaining the window's signal bit */

			IIntuition->GetAttr(WINDOW_Window, m_poWindowObj, (ULONG *) &m_poWindow);

			/* Indicate success */

			RetVal = KErrNone;

			/* Calculate the inner width and height of the window, for l8r use */

			m_iInnerWidth = (m_poWindow->Width - (m_poWindow->BorderRight + m_poWindow->BorderLeft));
			m_iInnerHeight = (m_poWindow->Height - (m_poWindow->BorderBottom + m_poWindow->BorderTop));
		}
		else
		{
			IIntuition->DisposeObject(m_poWindowObj);
			m_poWindowObj = NULL;

			Utils::Info("Unable to open window");
		}
	}
	else
	{
		Utils::Info("Unable to create window");
	}

#else /* ! __amigaos4__ */

	HINSTANCE Instance;
	RECT Rect;
	WNDCLASS WndClass;

	(void) a_pccPubScreenName;

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
	WndClass.lpszMenuName = MAKEINTRESOURCE(101);
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

#endif /* ! __amigaos4__ */

	/* If everything went well, perform general postamble window opening work */

	if (RetVal == KErrNone)
	{
		CompleteOpen();
	}

	/* Otherwise clean up whatever resources were allocated */

	else
	{
		// TODO: CAW - Unregister on error
		Close();
	}

	return(RetVal);
}

/* Written: Monday 08-Feb-2010 7:18 am */

void CWindow::Close()
{
	CStdGadget *Gadget;

#ifdef __amigaos4__

	if (m_poWindowObj)
	{
		IIntuition->DisposeObject(m_poWindowObj);
		m_poWindowObj = NULL;
	}

#else /* ! __amigaos4__ */

	if (m_poWindow)
	{

#ifdef _DEBUG

		ASSERTM((DestroyWindow(m_poWindow) != 0), "CWindow::Close() => Unable to destroy window");

#else /* ! _DEBUG */

		DestroyWindow(m_poWindow);

#endif /* ! _DEBUG */

		m_poWindow = NULL;
	}

#endif /* ! __amigaos4__ */

	/* Iterate through the list of attached gadgets, remove them from the gadget list and delete them */

	while ((Gadget = m_oGadgets.RemHead()) != NULL)
	{
		delete Gadget;
	}

	/* And remove the window from the application's list of windows, but only */
	/* if it was added */

	if (m_bOpen)
	{
		m_poApplication->RemoveWindow(this);
		m_bOpen = EFalse;
	}
}

/* Written: Wednesday 13-Oct-2010 7:01 am*/

void CWindow::CompleteOpen()
{

#ifdef __amigaos4__

	ASSERTM(m_poWindowObj, "CWindow::CompleteOpen() => Reaction window must be already open");

	/* Get the window's signal so that we can wait until an event occurs */

	IIntuition->GetAttr(WINDOW_Window, m_poWindowObj, (ULONG *) &m_poWindow);

#endif /* __amigaos4__ */

	/* Add this dialog to the application so that messages can be routed appropriately */

	m_poApplication->AddWindow(this);
	m_bOpen = ETrue;
}

/* Written: Saturday 29-May-2010 1:07 pm*/

void CWindow::DrawNow()
{

#ifdef __amigaos4__

	/* Fill the window background with the standard background colour.  The IIntuition->ShadeRect() */
	/* function is passed the inclusive right and bottom offsets to which to draw, not the size of */
	/* the rect to draw.  We only fill the background if required to as client code can disable */
	/* this functionality */

	if (m_bFillBackground)
	{
		IIntuition->ShadeRect(m_poWindow->RPort, m_poWindow->BorderLeft, m_poWindow->BorderTop,
			(m_poWindow->BorderLeft + m_iInnerWidth - 1), (m_poWindow->BorderTop + m_iInnerHeight - 1),
			LEVEL_NORMAL, BT_BACKGROUND, IDS_NORMAL, IIntuition->GetScreenDrawInfo(m_poWindow->WScreen), TAG_DONE);
	}

	/* And call the derived rendering function to perform a redraw immediately */

	Draw();

#else /* ! __amigaos4__ */

	/* Invalidate the client rect and redraw it, automatically filling the background with the */
	/* background colour first.  If this fails then there isn't much we can do besides ignore */
	/* the error */

	// TODO: CAW - Ensure that m_bFillBackground is taken into account
	InvalidateRect(m_poWindow, NULL, TRUE);

#endif /* ! __amigaos4__ */

}

#ifdef __amigaos4__

/* Written: Saturday 06-Nov-2010 8:27 am */

struct Screen *CWindow::GetRootWindowScreen()
{
	struct Window *Window;

	/* Get a ptr to the root window's underlying Amiga OS window, taking into account that there */
	/* may not be a root window */

	Window = (m_poRootWindow) ? m_poRootWindow->m_poWindow : NULL;

	/* If there was a root window then return that window's Screen */

	return((Window) ? Window->WScreen : NULL);
}

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
