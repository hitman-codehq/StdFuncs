
#include <StdFuncs.h>
#include <StdList.h>
#include <Test.h>
#include <string.h>

static RTest Test("T_StdList");	/* Class to use for testing and reporting results */

#define NUM_NODES 10

static const char *g_apccUnsortedStrings[NUM_NODES] =
{
	"One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten"
};

static const char *g_apccDoubleUnsortedStrings[NUM_NODES * 2] =
{
	"One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten",
	"One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten"
};

static const char *g_apccSortedStrings[NUM_NODES] =
{
	"Eight", "Five", "Four", "Nine", "One", "Seven", "Six", "Ten", "Three", "Two"
};

class CNode
{
public:

	const char	*m_pccName;	/* Ptr to the name of the node */

public:

	CNode(const char *a_pccName)
	{
		m_pccName = a_pccName;
	}

	StdListNode<CNode>	m_oStdListNode;		/* Standard list node */
};

/* Written: Sunday 08-Jun-2014 12:16 pm, on train to Ammersee */

static void CheckList(StdList<CNode> &a_roList, const char **a_ppccStrings, TInt a_iNumStrings)
{
	TInt Index;
	CNode *Node;

	test(a_roList.Count() == a_iNumStrings);

	Index = 0;
	Node = a_roList.getHead();
	test(Node != NULL);

	while (Node)
	{
		test(strcmp(Node->m_pccName, a_ppccStrings[Index]) == 0);

		++Index;
		Node = a_roList.getSucc(Node);
	}

	test(Index == a_iNumStrings);
}

/* Written: Wednesday 09-Jul-2014 6:29 am, Code HQ Ehinger Tor */

static TInt CompareNodes(const CNode *a_poFirst, const CNode *a_poSecond, void *a_pvUserData)
{
	(void) a_pvUserData;

	return strcmp(a_poFirst->m_pccName, a_poSecond->m_pccName);
}

/* Written: Sunday 08-Jun-2014 12:21 pm, on train to Ammersee */

static void CreateList(StdList<CNode> &a_roList, const char **a_ppccStrings)
{
	TInt Index;
	CNode *Node;

	for (Index = 0; Index < NUM_NODES; ++Index)
	{
		Node = new CNode(a_ppccStrings[Index]);
		test(Node != NULL);

		a_roList.addTail(Node);
	}
}

/* Written: Sunday 08-Jun-2014 12:04 pm, on train to Ammersee */

static void FreeList(StdList<CNode> &a_roList)
{
	CNode *Node;

	while ((Node = a_roList.remHead()) != NULL)
	{
		delete Node;
	}
}

/* Written: Friday 13-Jun-2014 7:40 am, Code HQ Ehinger Tor */

static void TestAppend()
{
	StdList<CNode> List, Source1, Source2;

	/* Test #2: Test that one list can be appended to the end of another list */

	Test.Next("Test that one list can be appended to the end of another list");

	CreateList(Source1, g_apccUnsortedStrings);
	CheckList(Source1, g_apccUnsortedStrings, NUM_NODES);

	CreateList(Source2, g_apccUnsortedStrings);
	CheckList(Source2, g_apccUnsortedStrings, NUM_NODES);

	List.AppendList(&Source1);
	CheckList(List, g_apccUnsortedStrings, NUM_NODES);

	List.AppendList(&Source2);
	CheckList(List, g_apccDoubleUnsortedStrings, (NUM_NODES * 2));

	test(Source1.Count() == 0);
	test(Source2.Count() == 0);
	test(List.Count() == NUM_NODES * 2);

	FreeList(List);
	test(List.Count() == 0);

	FreeList(Source1);
	test(Source1.Count() == 0);

	FreeList(Source2);
	test(Source2.Count() == 0);

	/* Try appending an empty list to another list, to ensure that the case is handled */

	List.AppendList(&Source1);
	test(List.Count() == 0);
	test(Source1.Count() == 0);
	FreeList(List);
}

/* Written: Saturday 07-Jan-2015 8:05 am, Code HQ Ehinger Tor */

static void TestIndex()
{
	TInt Index;
	CNode *Node;
	StdList<CNode> List;

	/* Test #5: Test array style indexing functionality */

	Test.Next("Test array style indexing functionality");

	/* Create a list containing NUM_NODES strings and index it by known string position */

	CreateList(List, g_apccUnsortedStrings);

	/* Ensure accessing out of bounds nodes fails */

	Node = List[-1];
	test(Node == NULL);

	Node = List[NUM_NODES];
	test(Node == NULL);

	/* Check that the first and last nodes match those returned by GetHead() and GetTail() */

	Node = List.getHead();
	test(Node != NULL);
	test(Node == List[0]);

	Node = List.getTail();
	test(Node != NULL);
	test(Node == List[NUM_NODES - 1]);

	/* And check the node in the middle is returned correctly */

	Node = List.getHead();
	test(Node != NULL);

	for (Index = 0; Index < 4; ++Index)
	{
		Node = List.getSucc(Node);
		test(Node != NULL);
	}

	test(Node == List[4]);

	FreeList(List);
}

/* Written: Sunday 08-Jun-2014 12:24 pm, on train to Ammersee */

static void TestMove()
{
	StdList<CNode> List, SourceList;

	/* Test #3: Test that nodes can be moved from one list to another */

	Test.Next("Test that nodes can be moved from one list to another");

	CreateList(SourceList, g_apccUnsortedStrings);
	CheckList(SourceList, g_apccUnsortedStrings, NUM_NODES);

	List.moveList(&SourceList);
	CheckList(List, g_apccUnsortedStrings, NUM_NODES);

	test(SourceList.Count() == 0);
	test(List.Count() == NUM_NODES);

	FreeList(List);
	test(List.Count() == 0);

	/* Try moving an empty list to another list, to ensure that the case is handled */

	List.moveList(&SourceList);
	test(List.Count() == 0);
	test(SourceList.Count() == 0);
	FreeList(List);
}

StdList<CNode> GetList()
{
	CNode *Node;
	StdList<CNode> RetVal;

	Node = new CNode("Move it!");
	test(Node != NULL);
	RetVal.addTail(Node);

	return RetVal;
}

/* Written: Thursday 29-Feb-2024 7:04 am, Code HQ Tokyo Tsukuda */

void TestMoveSemantics()
{
	CNode *Node;
	StdList<CNode> List1, Source1, Source2, Source3, Source4, Source5;

	Test.Next("Test C++ move constructor semantics are respected");

	List1 = std::move(Source1);
	test(Source1.m_oHead.m_poNext == &Source1.m_oTail);
	test(Source1.m_oTail.m_poPrev == &Source1.m_oHead);
	test(Source1.Count() == 0);
	test(List1.m_oHead.m_poNext == &List1.m_oTail);
	test(List1.m_oTail.m_poPrev == &List1.m_oHead);
	test(List1.Count() == 0);

	Node = new CNode("Move it!");
	test(Node != NULL);
	Source1.addTail(Node);

	List1 = std::move(Source1);
	test(Source1.m_oHead.m_poNext == &Source1.m_oTail);
	test(Source1.m_oTail.m_poPrev == &Source1.m_oHead);
	test(Source1.Count() == 0);
	test(List1.m_oHead.m_poNext == &Node->m_oStdListNode);
	test(List1.m_oTail.m_poPrev == &Node->m_oStdListNode);
	test(Node->m_oStdListNode.m_poPrev == &List1.m_oHead);
	test(Node->m_oStdListNode.m_poNext == &List1.m_oTail);
	test(List1.Count() == 1);

	Test.Next("Test C++ move assignment operator semantics are respected");

	StdList<CNode> List2 = std::move(Source2);
	test(Source2.m_oHead.m_poNext == &Source2.m_oTail);
	test(Source2.m_oTail.m_poPrev == &Source2.m_oHead);
	test(Source2.Count() == 0);
	test(List2.m_oHead.m_poNext == &List2.m_oTail);
	test(List2.m_oTail.m_poPrev == &List2.m_oHead);
	test(List2.Count() == 0);

	Source3.addTail(Node);

	StdList<CNode> List3(std::move(Source3));
	test(Source3.m_oHead.m_poNext == &Source3.m_oTail);
	test(Source3.m_oTail.m_poPrev == &Source3.m_oHead);
	test(Source3.Count() == 0);
	test(List3.m_oHead.m_poNext == &Node->m_oStdListNode);
	test(List3.m_oTail.m_poPrev == &Node->m_oStdListNode);
	test(Node->m_oStdListNode.m_poPrev == &List3.m_oHead);
	test(Node->m_oStdListNode.m_poNext == &List3.m_oTail);
	test(List3.Count() == 1);

	Test.Next("Test StdList.remHead() functions when move semantics are used");

	Source4.addTail(Node);

	StdList<CNode> List4(std::move(Source4));

	FreeList(List4);
	test(List4.m_oHead.m_poNext == &List4.m_oTail);
	test(List4.m_oTail.m_poPrev == &List4.m_oHead);
	test(List4.Count() == 0);

	Test.Next("Test StdList.getHead() functions when move semantics are used");

	Node = new CNode("Move it!");
	test(Node != NULL);
	Source5.addTail(Node);

	StdList<CNode> List5(std::move(Source5));

	while ((Node = List5.getHead()) != NULL)
	{
		List5.remove(Node);
		delete Node;
	}

	test(List5.m_oHead.m_poNext == &List5.m_oTail);
	test(List5.m_oTail.m_poPrev == &List5.m_oHead);
	test(List5.Count() == 0);

	Test.Next("Test returning a list instance by value when move semantics are used");

	StdList<CNode> List6 = GetList();
	test(List6.m_oHead.m_poNext != &List6.m_oTail);
	test(List6.m_oTail.m_poPrev != &List6.m_oHead);
	test(List6.m_oHead.m_poNext->m_poPrev == &List6.m_oHead);
	test(List6.m_oTail.m_poPrev->m_poNext == &List6.m_oTail);
	test(List6.Count() == 1);

	FreeList(List6);
}

/* Written: Friday 27-Jun-2014 7:12 am, Code HQ Ehinger Tor */

static void TestSort()
{
	StdList<CNode> List;

	/* Test #4: Test sorting a list alphabetically */

	Test.Next("Test sorting a list alphabetically");

	/* First ensure that an empty list can be handled */

	List.Sort(CompareNodes, NULL);

	/* Create a list containing NUM_NODES unsorted strings, sort it and ensure that it is sorted as expected */

	CreateList(List, g_apccUnsortedStrings);
	CheckList(List, g_apccUnsortedStrings, NUM_NODES);

	List.Sort(CompareNodes, NULL);

	CheckList(List, g_apccSortedStrings, NUM_NODES);

	FreeList(List);
}

int main()
{
	Test.Title();
	Test.Start("RStdList class API test");

	TestAppend();
	TestMove();
	TestSort();
	TestIndex();
	TestMoveSemantics();

	Test.End();

	return(RETURN_OK);
}
