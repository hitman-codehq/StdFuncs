
#ifndef STDDIALOG_H
#define STDDIALOG_H

#include "StdWindow.h"

/* Forward declaration to avoid circular dependency */

class CDialog;

/* Event types that can be sent to CDialog::HandleEvent() */

enum TStdEventType
{
	EStdEventChange = 0x0300		/**< Gadget contents have changed */
};

#ifdef __amigaos__

/* Each instance of this structure represents an Amiga OS gadget mapping */

struct SStdGadgetMapping
{
	Object	*m_poGadget;			/**< Ptr to BOOPSI gadget */
	TInt	m_iID;					/**< Integer ID of the gadget */
};

#elif defined(QT_GUI_LIB)

/* Each instance of this structure represents a Qt gadget mapping.  Qt gadget names are */
/* specified using strings but The Framework uses integers, so this allows us to map */
/* the one onto the other */

struct SStdGadgetMapping
{
	const char	*m_pccName;			/**< ID of the gadget, represented as a string */
	int			m_iID;				/**< Integer ID of the gadget */
};

#endif /* QT_GUI_LIB */

/* Mixin class for notifying client code that a dialog event has occurred */

class MDialogObserver
{
public:

	virtual void HandleDialogCommand(CDialog *a_poDialog, TInt a_iGadgetID) = 0;
};

/* This is the base class for all platform specific dialog boxes */

class CDialog : public CWindow
{
protected:

	char					*m_pcTextBuffer;		/**< Scratch buffer containing last obtained text */
	TInt					m_iTextBufferSize;		/**< Size of scratch buffer, in bytes */
	TInt					m_iNumGadgetMappings;	/**< Number of entries in m_poGadgetMappings */
	MDialogObserver			*m_poDialogObserver;	/**< Ptr to dialog's observer to notify of events */

#ifdef __amigaos__

	struct SStdGadgetMapping *m_poGadgetMappings;	/**< Array of gadget ID -> APTR mappings */
	struct SGWork			*m_poEditHookData;		/**< If non NULL, a string gadget is being edited */
	Object					*m_poRootGadget;		/**< Ptr to root layout gadget */

#elif defined(QT_GUI_LIB)

	const SStdGadgetMapping	*m_pcoGadgetMappings;	/**< Array of gadget names -> gadget ID mappings */
	QWidget					*m_poDialog;			/**< Pointer to the underlying Qt dialog */

#endif /* QT_GUI_LIB */

private:

	CDialog() { }

protected:

	virtual TInt DoOpen(TInt a_ResourceID);

	virtual void close();

	void close(TInt a_iGadgetID);

	void EnableGadget(TInt a_iGadgetID, TBool a_bEnable);

#ifdef __amigaos__

	Object *GetBOOPSIGadget(TInt a_iGadgetID);

#endif /* __amigaos__ */

	TInt GetGadgetInt(TInt a_iGadgetID);

	TInt GetGadgetText(TInt a_iGadgetID, TBool a_bGetText);

	void CheckGadget(TInt a_iGadgetID);

	void HighlightGadgetText(TInt a_iGadgetID);

	TBool IsGadgetChecked(TInt a_iGadgetID);

	void SetGadgetFocus(TInt a_iGadgetID);

	void SetGadgetText(TInt a_iGadgetID, const char *a_pccText);

#ifdef QT_GUI_LIB

	QWidget	*GetQtWidget(int a_iGadgetID);

	void SetMappingTable(const struct SStdGadgetMapping *a_pcoMappings, int a_iNumMappings);

#endif /* QT_GUI_LIB */

public:

	CDialog(RApplication *a_poApplication, MDialogObserver *a_poDialogObserver = NULL)
		: CWindow(a_poApplication), m_poDialogObserver(a_poDialogObserver)
	{
		/* Dialogs don't have their backgrounds filled */

		EnableFillBackground(EFalse);
	}

	virtual ~CDialog();

#ifdef __amigaos__

	static TInt	StringEditHook(struct Hook *a_poHook, struct SGWork *a_poWork, TUint *a_pulMessage);

	TInt DoStringEditHook(struct Hook *a_poHook, struct SGWork *a_poWork, TUint *a_pulMessage);

#endif /* __amigaos__ */

	/* Functions that can be implemented by client software */

	virtual void HandleEvent(enum TStdEventType /*a_eEventID*/, TInt /*a_iGadgetID*/) { }

	virtual void InitDialog() { }

	virtual TBool OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_bKeyDown*/);
};

#endif /* ! STDDIALOG_H */
