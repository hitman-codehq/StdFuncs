
#include <StdFuncs.h>
#include <StdList.h>
#include <Test.h>

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
	Node = a_roList.GetHead();
	test(Node != NULL);

	while (Node)
	{
		test(strcmp(Node->m_pccName, a_ppccStrings[Index]) == 0);

		++Index;
		Node = a_roList.GetSucc(Node);
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

		a_roList.AddTail(Node);
	}
}

/* Written: Sunday 08-Jun-2014 12:04 pm, on train to Ammersee */

static void FreeList(StdList<CNode> &a_roList)
{
	CNode *Node;

	while ((Node = a_roList.RemHead()) != NULL)
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

	/* Test #6: Test array style indexing functionality */

	Test.Next("Test array style indexing functionality");

	/* Create a list containing NUM_NODES strings and index it by known string position */

	CreateList(List, g_apccUnsortedStrings);

	/* Ensure accessing out of bounds nodes fails */

	Node = List[-1];
	test(Node == NULL);

	Node = List[NUM_NODES];
	test(Node == NULL);

	/* Check that the first and last nodes match those returned by GetHead() and GetTail() */

	Node = List.GetHead();
	test(Node != NULL);
	test(Node == List[0]);

	Node = List.GetTail();
	test(Node != NULL);
	test(Node == List[NUM_NODES - 1]);

	/* And check the node in the middle is returned correctly */

	Node = List.GetHead();
	test(Node != NULL);

	for (Index = 0; Index < 4; ++Index)
	{
		Node = List.GetSucc(Node);
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

	List.MoveList(&SourceList);
	CheckList(List, g_apccUnsortedStrings, NUM_NODES);

	test(SourceList.Count() == 0);
	test(List.Count() == NUM_NODES);

	FreeList(List);
	test(List.Count() == 0);

	/* Try moving an empty list to another list, to ensure that the case is handled */

	List.MoveList(&SourceList);
	test(List.Count() == 0);
	test(SourceList.Count() == 0);
	FreeList(List);
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

	Test.End();

	return(RETURN_OK);
}
