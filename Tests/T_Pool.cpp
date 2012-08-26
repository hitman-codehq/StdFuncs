
#include <StdFuncs.h>
#include <Test.h>
#include <StdPool.h>

#define NUM_NODES 500

static RTest Test("T_Pool");		/* Class to use for testing and reporting results */

struct SPoolItem
{
	StdListNode<SPoolItem>	m_oStdListNode;		/* Standard list node */
	int	m_iOne;
};

static void AllocateNodes(RStdPool *a_poPool, int a_iNumNodes, StdList<SPoolItem> *a_poNodes)
{
	int Index;
	struct SPoolItem *PoolItem;

	for (Index = 0; Index < a_iNumNodes; ++Index)
	{
		PoolItem = (struct SPoolItem *) a_poPool->GetNode();
		test(PoolItem != NULL);

		PoolItem->m_iOne = ((Index << 16) | Index);
		a_poNodes->AddTail(PoolItem);
	}
}

static void FreeNodes(RStdPool *a_poPool, StdList<SPoolItem> *a_poNodes)
{
	int Index;
	struct SPoolItem *PoolItem;

	for (Index = 0; Index < NUM_NODES; ++Index)
	{
		PoolItem = a_poNodes->GetHead();
		test(PoolItem != NULL);
		test(PoolItem->m_iOne == ((Index << 16) | Index));

		a_poNodes->Remove(PoolItem);
	}
}

int main()
{
	int Index, Result;
	struct SPoolItem *PoolItem;
	StdList<SPoolItem> Nodes;

	Test.Title();
	Test.Start("RPool class API test");

	RStdPool Pool;

	/* Test #1: Create a pool that has a maximum number of nodes and which cannot */
	/* be extended */

	Test.Next("Create a non extensible pool");
	Result = Pool.Create(sizeof(struct SPoolItem), NUM_NODES, EFalse);
	test(Result == KErrNone);

	Pool.Close(ETrue);
	test(Pool.Count() == 0);

	/* Test #2: Try to call RStdPool::Create() twice in a row */

	Test.Next("Try to call RStdPool::Create() twice in a row");
	Result = Pool.Create(sizeof(struct SPoolItem), NUM_NODES, EFalse);
	test(Result == KErrNone);

	Result = Pool.Create(sizeof(struct SPoolItem), NUM_NODES, EFalse);
	test(Result == KErrInUse);

	Pool.Close(ETrue);
	test(Pool.Count() == 0);

	/* Test #3: Create a non extensible pool and allocate all nodes */

	Test.Next("Create a non extensible pool and allocate all nodes");
	Result = Pool.Create(sizeof(struct SPoolItem), NUM_NODES, EFalse);
	test(Result == KErrNone);

	/* Allocate the nodes */

	AllocateNodes(&Pool, NUM_NODES, &Nodes);

	/* Ensure that when we ask for one more node, it fails */

	PoolItem = (struct SPoolItem *) Pool.GetNode();
	test(PoolItem == NULL);

	/* Free all of the allocated nodes */

	FreeNodes(&Pool, &Nodes);
	test(Pool.Count() == 0);

	/* Ensure that our temporary node list is now empty */

	PoolItem = Nodes.GetHead();
	test(PoolItem == NULL);

	Pool.Close(ETrue);
	test(Pool.Count() == 0);

	/* Test #4: Create an extensible pool and allocate first block of nodes */

	Test.Next("Create an extensible pool and allocate first block of nodes");
	Result = Pool.Create(sizeof(struct SPoolItem), NUM_NODES, ETrue);
	test(Result == KErrNone);

	/* Allocate the nodes */

	AllocateNodes(&Pool, NUM_NODES, &Nodes);

	/* Ensure that when we ask for one more node, it succeeds */

	PoolItem = (struct SPoolItem *) Pool.GetNode();
	test(PoolItem != NULL);
	test(Pool.Count() == (NUM_NODES - 1));

	/* Let the pool free the nodes this time and ensure that it succeeds */

	Pool.Close(ETrue);
	test(Pool.Count() == 0);

	/* The node list contents are no longer valid so hard reset the list */

	Nodes.Reset();
	test(Nodes.Count() == 0);

	/* Test #5: Create a non extensible pool and allocate, free and reallocate all nodes */

	Test.Next("Create a non extensible pool and allocate all nodes");
	Result = Pool.Create(sizeof(struct SPoolItem), NUM_NODES, EFalse);
	test(Result == KErrNone);

	// TODO: CAW - Temporarily 1 until we actually release the nodes!
	for (Index = 0; Index < 1; ++Index)
	{
		/* Allocate the nodes */

		AllocateNodes(&Pool, NUM_NODES, &Nodes);

		/* Ensure that when we ask for one more node, it fails */

		PoolItem = (struct SPoolItem *) Pool.GetNode();
		test(PoolItem == NULL);

		/* Free all of the nodes in preparation for reallocating them */

		FreeNodes(&Pool, &Nodes);
	}

	Test.End();

	return(RETURN_OK);
}
