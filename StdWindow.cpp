
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
	{ STD_KEY_DELETE, VK_DELETE }, { STD_KEY_TAB, VK_TAB }, { STD_KEY_F3, VK_F3 },
	{ STD_KEY_F12, VK_F12 }
};

#define NUM_KEYMAPPINGS 16

TBool CWindow::m_bCtrlPressed;

#endif /* ! __amigaos4__ */

CWindow *CWindow::m_poRootWindow; /* Ptr to root window on which all other windows open */

#ifdef __amigaos4__

/* Written: Saturday 20-Nov-2010 11:05 am */

void CWindow::IDCMPFunction(struct Hook *a_poHook, Object * /*a_poObject*/, struct IntuiMessage *a_poIntuiMessage)
{
	struct IntuiWheelData *IntuiWheelData;
	struct TagItem *TagItem;
	CStdGadget *Gadget;
	CWindow *Window;

	/* Get a ptr to the Window associated with the hook */

	Window = (CWindow *) a_poHook->h_Data;

	/* If this is a mouse wheel event then convert the delta to a Windows style reading */
	/* (+/- 120 per notch) and notify the window */

	if (a_poIntuiMessage->Class == IDCMP_EXTENDEDMOUSE)
	{
		IntuiWheelData = (struct IntuiWheelData *) a_poIntuiMessage->IAddress;
		Window->HandleWheelEvent(-IntuiWheelData->WheelY * 120);
	}

	/* If this is a message from a BOOPSI object saying that it has been updated, find the object and */
	/* map it onto its matching gadget and call the gadget's Updated() function */

	else if (a_poIntuiMessage->Class == IDCMP_IDCMPUPDATE)
	{
		/* Get the gadget's unique ID */

		if ((TagItem = IUtility->FindTagItem(GA_ID, (struct TagItem *) a_poIntuiMessage->IAddress)) != NULL)
		{
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
		case WM_ACTIVATE :
		{
			/* If window focus is changing then reset the state of the ctrl key, as we don't */
			/* get the WM_KEYUP event if the window is not active.  Also let client software */
			/* know about this */

			if (m_bCtrlPressed)
			{
				// TODO: CAW - Should really use an Active() function
				Window->OfferKeyEvent(STD_KEY_CONTROL, EFalse);
			}

			m_bCtrlPressed = EFalse;

			break;
		}

		case WM_CLOSE :
		{
			/* Allow the client window itself to handle the close */

			Window->HandleCommand(IDCANCEL);

			break;
		}

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
			/* If the ctrl key is currently pressed then convert the keycode back to standard ASCII */

			if (m_bCtrlPressed)
			{
				a_oWParam |= 0x60;
			}

			/* Call the CWindow::OfferKeyEvent() function, passing in only valid ASCII characters */

			if ((a_oWParam >= 32) && (a_oWParam <= 254) && (a_oWParam != 127))
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

			/* This is pretty horrible.  Because Windows mixes its WM_KEY#? and WM_CHAR events, we */
			/* can get duplicate keys that need to get filtered out in order to provide a consistent */
			/* pattern to calling CWindow::OfferKeyEvent() .  Also, when ctrl is pressed the ASCII */
			/* characters sent to WM_CHAR messages are different so again we need to adjust these */
			/* back to standard ASCII so keeping track of the state of the ctrl key is the only way */
			/* to achieve this */

			if (a_oWParam == VK_CONTROL)
			{
				m_bCtrlPressed = (a_uiMessage == WM_KEYDOWN) ? ETrue : EFalse;
			}

			/* This is even more horrible.  When the ALT GR key is pressed, rather than using a special */
			/* key, Windows sends through a VK_CONTROL WM_KEYDOWN event, followed by WM_MENU (otherwise */
			/* known as ALT) WM_KEYDOWN and WM_KEYUP events, with NO following VK_CONTROL WM_KEYUP event! */
			/* So we need to put some special magic in here to handle this nonsense or we will think that */
			/* the ctrl key is pressed when it isn't */

			else if (a_oWParam == VK_MENU)
			{
				if ((a_uiMessage == WM_KEYUP) && (m_bCtrlPressed))
				{
					m_bCtrlPressed = EFalse;
				}
			}

			break;
		}

		case WM_LBUTTONDOWN :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam), EStdMouseDown);

			break;
		}

		case WM_LBUTTONUP :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam), EStdMouseUp);

			break;
		}

		case WM_LBUTTONDBLCLK :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam), EStdMouseDoubleClick);

			break;
		}

		case WM_MOUSEMOVE :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam), EStdMouseMove);

			break;
		}

		case WM_MOUSEWHEEL :
		{
			/* Extract the 16 bit amount the wheel has been scrolled and ensure that it is signed */

			Window->HandleWheelEvent((short) HIWORD(a_oWParam));

			break;
		}

		case WM_PAINT :
		{
			/* Prepare the device context for painting and call the CWindow::Draw() routine. */
			/* If this fails then there isn't much we can do besides ignore the error */

			if ((Window->m_poDC = BeginPaint(a_poWindow, &Window->m_oPaintStruct)) != NULL)
			{
				Window->Draw(Window->m_oPaintStruct.rcPaint.top, Window->m_oPaintStruct.rcPaint.bottom);
				EndPaint(a_poWindow, &Window->m_oPaintStruct);
			}

			break;
		}

		case WM_SIZE :
		{
			Window->InternalResize(LOWORD(a_oLParam), HIWORD(a_oLParam));

			break;
		}

		case WM_HSCROLL :
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
	IIntuition->SetWindowAttrs(m_poWindow, WA_Activate, TRUE, TAG_DONE);

#else /* ! __amigaos4__ */

	DEBUGCHECK((SetActiveWindow(m_poWindow) != NULL), "CWindow::Activate() => Unable to activate window");

#endif /* ! __amigaos4__ */

}

/* Written: Sunday 21-Nov-2010 8:11 am */

void CWindow::Attach(CStdGadget *a_poGadget)
{
	ASSERTM((a_poGadget != NULL), "CWindow::Attach() => No gadget to be attached passed in");
	ASSERTM((m_poWindow != NULL), "CWindow::Attach() => Window not yet open");

#ifdef __amigaos4__

	/* Add the new BOOPSI gadget to the window's root layout */

	if (IIntuition->IDoMethod(m_poRootGadget, LM_ADDCHILD, NULL, a_poGadget->m_poGadget, NULL))
	{
		ILayout->RethinkLayout((struct Gadget *) m_poRootGadget, m_poWindow, NULL, TRUE);
	}

#endif /* __amigaos4__ */

	/* This is - ahem - a little hacky as it assumes that the gadget being attached is of a certain */
	/* type!  We'll sort out doing this properly when the time comes */

	if (a_poGadget->GadgetType() == EStdGadgetVerticalSlider)
	{
		m_iInnerWidth -= a_poGadget->Width();
	}

	if ((a_poGadget->GadgetType() == EStdGadgetHorizontalSlider) || (a_poGadget->GadgetType() == EStdGadgetStatusBar))
	{
		m_iInnerHeight -= a_poGadget->Height();
	}

	m_oGadgets.AddTail(a_poGadget);
}

/* Written: Sunday 01-May-2011 10:48 am */
/* @param	a_iInnerWidth	New width of client area */
/*			a_iInnerHeight	New height of client area */
/* This function is called whenever the size of the window changes and will reposition any auto-position */
/* gadgets and will notify the derived window class that the size has changed. */

void CWindow::InternalResize(TInt a_iInnerWidth, TInt a_iInnerHeight)
{
	TInt OldInnerWidth, OldInnerHeight;
	CStdGadget *Gadget;

	/* Save the old width & height for l8r */

	OldInnerWidth = m_iInnerWidth;
	OldInnerHeight = m_iInnerHeight;

	/* Save the new width & height for l8r */

	m_iInnerWidth = a_iInnerWidth;
	m_iInnerHeight = a_iInnerHeight;

	/* Iterate through the gadgets and reposition them to reflect the new window size, also */
	/* adjusting the size of the client area of the window to reflect the space taken up by them */

	Gadget = m_oGadgets.GetHead();

	while (Gadget)
	{
		/* For Windows the status bar needs a special message to be sent to it to let it know that */
		/* the window's size has changed (even though the documentation indicates that this is not */
		/* necessary) */

		if (Gadget->GadgetType() == EStdGadgetStatusBar)
		{
			m_iInnerHeight -= Gadget->Height();

#ifndef __amigaos4__

			SendMessage(Gadget->m_poGadget, WM_SIZE, 0, 0);

#endif /* ! __amigaos4__ */

		}

		/* Vertical sliders always go on the right (for now) and are the hight of the client area */

		if (Gadget->GadgetType() == EStdGadgetVerticalSlider)
		{
			m_iInnerWidth -= Gadget->Width();
			MoveWindow(Gadget->m_poGadget, m_iInnerWidth, 0, Gadget->Width(), m_iInnerHeight, TRUE);
		}

		/* Horizontal sliders always go on the bottom (for now) and are the width of the client area */

		if (Gadget->GadgetType() == EStdGadgetHorizontalSlider)
		{
			m_iInnerHeight -= Gadget->Height();
			MoveWindow(Gadget->m_poGadget, 0, m_iInnerHeight, m_iInnerWidth, Gadget->Height(), TRUE);
		}

		Gadget = m_oGadgets.GetSucc(Gadget);
	}

	/* Don't let the client area get negative in size or it will require too much fiddling around */
	/* in the client program.  Just pretend it's 0 x 0 at the smallest */

	if (m_iInnerWidth < 0)
	{
		m_iInnerWidth = 0;
	}

	if (m_iInnerHeight < 0)
	{
		m_iInnerHeight = 0;
	}

	/* Now let the derived window class know that the window's size has changed */

	Resize(OldInnerWidth, OldInnerHeight);
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
		WINDOW_IDCMPHook, &m_oIDCMPHook, WINDOW_IDCMPHookBits, (IDCMP_EXTENDEDMOUSE | IDCMP_IDCMPUPDATE),
		WA_IDCMP, (IDCMP_CLOSEWINDOW | IDCMP_EXTENDEDMOUSE | IDCMP_IDCMPUPDATE | IDCMP_MENUPICK | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY | IDCMP_REFRESHWINDOW),

		WINDOW_ParentGroup, m_poRootGadget = (Object *) VGroupObject,
			/* This is an empty group into which can be placed BOOPSI objects */

			LAYOUT_HorizAlignment, LALIGN_RIGHT,
		EndGroup,
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
	WndClass.style = CS_DBLCLKS;
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

				/* And save the size of the client area */

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
		DEBUGCHECK((DestroyWindow(m_poWindow) != 0), "CWindow::Close() => Unable to destroy window");
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
	/* Just determine the dimensions of the window and pass the call on */

#ifdef __amigaos4__

	DrawNow(0, (m_poWindow->BorderTop + m_iInnerHeight));

#else /* ! __amigaos4__ */

	RECT Rect;

	/* Get the dimensions of the client area and adjust it to only represent the vertical */
	/* band that we wish to redraw, also adjusting the size of the area to be cleared and */
	/* drawn to take into account any attached gadgets */

	if (GetClientRect(m_poWindow, &Rect))
	{
		DrawNow(0, Rect.bottom);
	}
	else
	{
		Utils::Info("CWindow::DrawNow() => Unable to obtain client window dimensions");
	}

#endif /* ! __amigaos4__ */

}

/* Written: Saturday 30-Nov-2010 9:15 pm */
/* @param	a_iTop		Offset from top of client area from which to invalidate */
/*			a_iBottom	Bottom most part of client area to which to invalidate */
/* Invalidates a vertical band of the client area and instigates a redraw of that area. */
/* The bottom of the area, represented by a_iBottom, is considered exclusive, so the area */
/* redrawn is between a_iTop and (a_iBottom - 1) */

void CWindow::DrawNow(TInt a_iTop, TInt a_iBottom)
{
	ASSERTM((a_iTop >= 0), "CWindow::DrawNow() => Y offset to draw from must not be negative");
	ASSERTM((a_iBottom >= 0), "CWindow::DrawNow() => Y offset to draw to must not be negative");
	ASSERTM((a_iTop <= a_iBottom), "CWindow::DrawNow() => Y offset start must be less than Y offset stop");

#ifdef __amigaos4__

	int Bottom, Top;

	/* Fill the window background with the standard background colour.  The IIntuition->ShadeRect() */
	/* function is passed the inclusive right and bottom offsets to which to draw, not the size of */
	/* the rect to draw.  We only fill the background if required to as client code can disable */
	/* this functionality */

	if (m_bFillBackground)
	{
		Top = (m_poWindow->BorderTop + a_iTop);
		Bottom = (m_poWindow->BorderTop + a_iBottom);

		/* If the bottom line is off the bottom of the client area then clip it to the bottom line */
		/* of the client area */

		if (Bottom > (m_poWindow->BorderTop + m_iInnerHeight - 1))
		{
			Bottom = (m_poWindow->BorderTop + m_iInnerHeight - 1);
		}

		/* Now fill in the background before drawing */

		IIntuition->ShadeRect(m_poWindow->RPort, m_poWindow->BorderLeft, Top,
			(m_poWindow->BorderLeft + m_iInnerWidth - 1), Bottom,
			LEVEL_NORMAL, BT_BACKGROUND, IDS_NORMAL, IIntuition->GetScreenDrawInfo(m_poWindow->WScreen), TAG_DONE);
	}

	/* And call the derived rendering function to perform a redraw immediately */

	Draw(a_iTop, a_iBottom);

#else /* ! __amigaos4__ */

	RECT Rect;

	/* Get the dimensions of the client area and adjust it to only represent the vertical */
	/* band that we wish to redraw, also adjusting the size of the area to be cleared and */
	/* drawn to take into account any attached gadgets */

	if (GetClientRect(m_poWindow, &Rect))
	{
		Rect.bottom = (Rect.top + a_iBottom);
		Rect.top += a_iTop;
		Rect.right = m_iInnerWidth;

		/* And invalidate the vertical band */

		InvalidateRect(m_poWindow, &Rect, m_bFillBackground);
	}
	else
	{
		Utils::Info("CWindow::DrawNow() => Unable to obtain client window dimensions");
	}

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
