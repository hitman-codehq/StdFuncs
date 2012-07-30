
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
	Pool.Close();

	/* Test #2: Create a pool and allocate all nodes it contains */

	Test.Next("Create a non extensible pool and allocate all nodees");
	Result = Pool.Create(sizeof(struct SPoolItem), NUM_NODES, EFalse);
	test(Result == KErrNone);

	for (Index = 0; Index < NUM_NODES; ++Index)
	{
		PoolItem = (struct SPoolItem *) Pool.GetNode();
		test(PoolItem != NULL);

		PoolItem->m_iOne = ((Index << 16) | Index);
		Nodes.AddTail(PoolItem);
	}

	/* Ensure that when we ask for one more node, it fails */

	PoolItem = (struct SPoolItem *) Pool.GetNode();
	test(PoolItem == NULL);

	/* Free all of the allocated nodes */

	for (Index = 0; Index < NUM_NODES; ++Index)
	{
		PoolItem = Nodes.GetHead();
		test(PoolItem != NULL);
		test(PoolItem->m_iOne == ((Index << 16) | Index));

		Nodes.Remove(PoolItem);
	}

	/* Ensure that our temporary node list is now empty */

	PoolItem = Nodes.GetHead();
	test(PoolItem == NULL);

	// TODO: CAW - What type of test to do here?

	Pool.Close();

	Test.End();

	return(RETURN_OK);
}
