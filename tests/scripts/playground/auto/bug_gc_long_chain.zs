#include "auto/test_runner.zs"

// Freeing an object frees the objects only it held. That used to recurse once per object, so
// dropping a long linked list overflowed the native stack and crashed the game.

class ChainNode
{
	ChainNode next;
}

int tree_dtor_order[5];
int tree_dtor_count = 0;

class TreeNode
{
	int name;
	TreeNode left;
	TreeNode right;

	TreeNode(int name, TreeNode left, TreeNode right)
	{
		this->name = name;
		this->left = left;
		this->right = right;
	}

	~TreeNode()
	{
		tree_dtor_order[tree_dtor_count++] = name;
	}
}

generic script bug_gc_long_chain
{
	void run()
	{
		Test::Init();

		// A Release build used to crash at about 65000 (with an 8 MB stack).
		DropChain(100000);

		// Destructors still run in the same order: an object's own first, then each object it
		// released (with everything that one released), in order.
		DropTree();
		Test::AssertEqual(tree_dtor_count, 5);
		for (int i = 0; i < 5; i++)
			Test::AssertEqual(tree_dtor_order[i], i + 1);

		Test::End();
	}

	void DropChain(int count)
	{
		ChainNode head = NULL;
		for (int i = 0; i < count; i++)
		{
			ChainNode node = new ChainNode();
			node->next = head;
			head = node;
		}
	}

	void DropTree()
	{
		TreeNode root = new TreeNode(1,
			new TreeNode(2, new TreeNode(3, NULL, NULL), new TreeNode(4, NULL, NULL)),
			new TreeNode(5, NULL, NULL));
	}
}
