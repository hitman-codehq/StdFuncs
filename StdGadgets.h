
#ifndef STDGADGETS_H
#define STDGADGETS_H

#ifdef __amigaos__

#include <intuition/classes.h>

#elif defined(QT_GUI_LIB)

#include "Qt/QtGadgetSlider.h"

#endif /* QT_GUI_LIB */

/* Forward declarations to reduce the # of includes required */

class CStdGadgetLayout;
class CWindow;
class MStdGadgetLayoutObserver;
class MStdGadgetSliderObserver;
class QGridLayout;
class QLabel;
class QWidget;

/* Types of gadgets that can be created */

enum TStdGadgetType
{
	EStdGadgetVerticalLayout,	/* Vertical dynamic layout gadget */
	EStdGadgetHorizontalLayout,	/* Horizontal dynamic layout gadget */
	EStdGadgetVerticalSlider,	/* Vertical scroller */
	EStdGadgetHorizontalSlider,	/* Horizontal scroller */
	EStdGadgetStatusBar			/* Status bar */
};

/* The abstract base class used for all gadgets */

class CStdGadget
{
protected:

	bool					m_bHidden;			/**< true if the gadget is hidden */
	TInt					m_iGadgetID;		/**< Unique ID of the gadget */
	TInt					m_iX;				/**< X and Y positions of the gadget, relative */
	TInt					m_iY;				/**< to the top left hand corner of the client area */
	TInt					m_iWidth;			/**< Width of the gadget in pixels */
	TInt					m_iHeight;			/**< Height of the gadget in pixels */
	TInt					m_iMinHeight;		/**< Minimum height of the gadget in pixels */
	CStdGadgetLayout		*m_poParentLayout;	/**< Ptr to CStdGadgetLayout that owns this gadget */
	CWindow					*m_poParentWindow;	/**< Ptr to window that owns this gadget */
	enum TStdGadgetType		m_iGadgetType;		/**< Type of gadget */

#ifdef __amigaos__

	Object					*m_poGadget;		/**< Ptr to underlying BOOPSI gadget */

#elif defined(QT_GUI_LIB)

	QWidget					*m_poGadget;		/**< Ptr to underlying Qt widget */

#elif defined(WIN32)

	HWND					m_poGadget;			/**< Ptr to the underlying Windows control */

#endif /* WIN32 */

private:

	void SetPosition(TInt a_iX, TInt a_iY);

protected:

	virtual void SetSize(TInt a_iWidth, TInt a_iHeight);

public:

	StdListNode<CStdGadget>	m_oStdListNode;		/**< Standard list node */

	virtual ~CStdGadget();

	TInt GadgetID()
	{
		return(m_iGadgetID);
	}

	enum TStdGadgetType GadgetType()
	{
		return(m_iGadgetType);
	}

	virtual void SetVisible(bool a_bVisible);

	bool Visible()
	{
		return(!m_bHidden);
	}

	virtual TInt X();

	virtual TInt Y();

	virtual TInt Width();

	virtual TInt Height();

	virtual TInt MinHeight();

	virtual void Updated(ULONG /*a_ulData*/ = 0) { }

	/* CStdGadgetLayout classe needs to be able to access this class's internals in order to */
	/* manage the gadgets' positions etc. */

	friend class CStdGadgetLayout;
};

/* A special gadget that can automatically layout other gadgets inside itself */

class CStdGadgetLayout : public CStdGadget
{
private:

	TInt					m_iWeight;				/**< Weight of the layout gadget */
	MStdGadgetLayoutObserver *m_poClient;			/**< Ptr to client to notify when gadget changes */
	StdList<CStdGadget>		m_oGadgets;				/**< List of gadgets manually added to the window */

#ifdef QT_GUI_LIB

	QGridLayout				*m_poLayout;			/**< Ptr to underlying Qt widget.
														 Usually this is stored in m_poGadget but unfortunately
														 QLayout derived objects do not derive from QWidget */

#endif /* QT_GUI_LIB */

private:

	CStdGadgetLayout(CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout, TBool a_bVertical,
		MStdGadgetLayoutObserver *a_poClient) : CStdGadget()
	{
		m_poParentWindow = a_poParentWindow;
		m_poParentLayout = a_poParentLayout;
		m_iGadgetType = (a_bVertical) ? EStdGadgetVerticalLayout : EStdGadgetHorizontalLayout;
		m_poClient = a_poClient;
		m_iWeight = 50;
	}

	TInt Construct();

	CStdGadget *FindNativeGadget(void *a_pvGadget);

public:

	StdListNode<CStdGadgetLayout>	m_oStdListNode;	/**< Standard list node */

	static CStdGadgetLayout *New(CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout, TBool a_bVertical = ETrue,
		MStdGadgetLayoutObserver *a_poClient = NULL);

	~CStdGadgetLayout();

	void Attach(CStdGadget *a_poGagdet);

	TInt GetSpacing();

#ifdef __amigaos__

	void ReAttach(CStdGadget *a_poGadget);

#endif /* __amigaos__ */

	void rethinkLayout();

	void SetWeight(TInt a_iWeight);

	TInt Weight()
	{
		return(m_iWeight);
	}

	/* From CStdGadget */

	virtual TInt X();

	virtual TInt Y();

	virtual TInt MinHeight();

	/* CWindow classe needs to be able to access this class's internals in order to map */
	/* native events onto standard cross platform events */

	friend class CWindow;
};

/* A class representing a slider or proportional gadget */

class CStdGadgetSlider : public CStdGadget
{
private:

	TInt		m_iMaxRange;					/**< Maximum X/Y position of the slider */
	TInt		m_iPageSize;					/**< Number of characters/lines/pixels per page */
	TInt		m_iPosition;					/**< The position of the slider knob within the slider */
	MStdGadgetSliderObserver *m_poClient;		/**< Ptr to client to notify when gadget changes */

#ifdef QT_GUI_LIB

	bool		m_bSettingValue;				/**< true if the slider's value is being changed */
	CQtGadgetSlider	m_oSlider;					/**< Helper class for listening for signals */

#endif /* QT_GUI_LIB */

private:

#ifdef QT_GUI_LIB

	CStdGadgetSlider(TBool a_bVertical, CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout, MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID)
		: m_oSlider(this)

#else /* ! QT_GUI_LIB */

	CStdGadgetSlider(TBool a_bVertical, CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout, MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID)

#endif /* ! QT_GUI_LIB */

	{
		m_iGadgetType = (a_bVertical) ? EStdGadgetVerticalSlider : EStdGadgetHorizontalSlider;
		m_poParentWindow = a_poParentWindow;
		m_poParentLayout = a_poParentLayout;
		m_poClient = a_poClient;
		m_iGadgetID = a_iGadgetID;

		/* Set the default position to the top/left of the slider, which is 1, not 0 */

		m_iPosition = 1;
	}

	TInt Construct();

	bool CreateNative();

public:

	static CStdGadgetSlider *New(CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout,
		MStdGadgetSliderObserver *a_poClient, TBool a_bVertical, TInt a_iGadgetID);

	~CStdGadgetSlider();

	void SetPosition(TInt a_iPosition);

	void SetRange(TInt a_iPageSize, TInt a_iMaxRange);

	/* From CStdGadget */

#ifdef __amigaos__

	void SetVisible(bool a_bVisible) override;

#endif /* __amigaos__ */

	void Updated(ULONG a_ulData);
};

/* A class representing a status bar gadget */

class CStdGadgetStatusBar : public CStdGadget
{
private:

	char		**m_ppcPartsText;				/**< Ptr to an array of ptrs to text strings */
	TInt		m_iNumParts;					/**< Number of parts within the gadget */

#ifdef __amigaos__

	Object		**m_poPartsGadgets;				/**< Ptr to an array of ptrs to parts labels */

#elif defined(QT_GUI_LIB)

	QLabel		**m_poPartsGadgets;				/**< Ptr to an array of ptrs to parts labels */

#else /* ! QT_GUI_LIB */

	TInt		*m_piPartsOffsets;				/**< Pointer to an array of offsets of the parts in
													 the status bar (as percentages) */

#endif /* ! QT_GUI_LIB */

private:

	CStdGadgetStatusBar(CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout, TInt a_iGadgetID)
	{
		m_iGadgetType = EStdGadgetStatusBar;
		m_poParentWindow = a_poParentWindow;
		m_poParentLayout = a_poParentLayout;
		m_iGadgetID = a_iGadgetID;
	}

	TInt Construct(TInt a_iNumParts, TInt *a_piPartsOffsets);

	void SetSize(TInt a_iWidth, TInt a_iHeight);

public:

	~CStdGadgetStatusBar();

	static CStdGadgetStatusBar *New(CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout, TInt a_iNumParts, TInt *a_piPartsOffsets, TInt a_iGadgetID);

	const char *GetText(TInt a_iPart);

	void SetText(TInt a_iPart, const char *a_pccText);
};

/* Mixin class for the slider or proportional gadget to be able to notify its client */
/* when it has been updated */

class MStdGadgetSliderObserver
{
public:

	virtual void SliderUpdated(CStdGadgetSlider *a_poGadget, TInt a_iPosition) = 0;
};

/* Mixin class for the layout gadget to be able to notify its client when it has been updated */

class MStdGadgetLayoutObserver
{
public:

	virtual void Resize() = 0;
};

#endif /* ! STDGADGETS_H */
