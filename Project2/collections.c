

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "collections.h"

static RbNode sentinelNode = {
	.parent = NULL,
	.left = NULL,
	.right = NULL,
	.color = BLACK,
	.key = NULL,
	.data = NULL
};

#define SENTINEL (&sentinelNode)


typedef struct {
	RbNode* node;
	RbNode* prev;
} RbFindResult;


/// <summary>
/// Simple ordered-tree search 
/// </summary>
static RbFindResult findRbTreeItemImpl(RbTree* tree, void* key)
{
	RbNode* node = tree->root;
	RbNode* prev = SENTINEL;

	while (node != SENTINEL)
	{
		prev = node;
		int relation = tree->keyComparer(node->key, key);
		if (relation < 0)
		{
			node = node->right;
		}
		else if (relation > 0)
		{
			node = node->left;
		}
		else
		{
			break;
		}
	}

	return (RbFindResult) {
		.node = node,
			.prev = prev
	};
}


///<summary>
/// RotateLeft
/// Rebalance the tree by rotating the nodes to the left
///</summary>
static void rotateLeft(RbTree* tree, RbNode* x)
{
	// pushing node x down and to the Left to balance the tree. x's Right child (y)
	// replaces x (since y > x), and y's Left child becomes x's Right child 
	// (since it's < y but > x).

	RbNode* y = x->right;           // get x's Right node, this becomes y

	// set x's Right link
	x->right = y->left;                   // y's Left child's becomes x's Right child

	// modify parents
	if (y->left != SENTINEL)
		y->left->parent = x;				// sets y's Left Parent to x

	if (y != SENTINEL)
		y->parent = x->parent;            // set y's Parent to x's Parent

	if (x->parent != NULL && x->parent != SENTINEL)
	{   // determine which side of it's Parent x was on
		if (x == x->parent->left)
			x->parent->left = y;          // set Left Parent to y
		else
			x->parent->right = y;         // set Right Parent to y
	}
	else
		tree->root = y;                     // at rbTree, set it to y

	// link x and y 
	y->left = x;                         // put x on y's Left 
	if (x != SENTINEL)                      // set y as x's Parent
		x->parent = y;
}

///<summary>
/// RotateRight
/// Rebalance the tree by rotating the nodes to the right
///</summary>
static void rotateRight(RbTree* tree, RbNode* x)
{
	// pushing node x down and to the Right to balance the tree. x's Left child (y)
	// replaces x (since x < y), and y's Right child becomes x's Left child 
	// (since it's < x but > y).

	RbNode* y = x->left;            // get x's Left node, this becomes y

	// set x's Right link
	x->left = y->right;                   // y's Right child becomes x's Left child

	// modify parents
	if (y->right != SENTINEL)
		y->right->parent = x;				// sets y's Right Parent to x

	if (y != SENTINEL)
		y->parent = x->parent;            // set y's Parent to x's Parent

	if (x->parent != NULL && x->parent != SENTINEL)               // NULL=rbTree, could also have used rbTree
	{   // determine which side of it's Parent x was on
		if (x == x->parent->right)
			x->parent->right = y;         // set Right Parent to y
		else
			x->parent->left = y;          // set Left Parent to y
	}
	else
		tree->root = y;                     // at rbTree, set it to y

	// link x and y 
	y->right = x;                        // put x on y's Right
	if (x != SENTINEL)              // set y as x's Parent
		x->parent = y;
}

///<summary>
/// RestoreAfterInsert
/// Additions to red-black trees usually destroy the red-black 
/// properties. Examine the tree and restore. Rotations are normally 
/// required to restore it
///</summary>
static void restoreAfterInsert(RbTree* tree, RbNode* x)
{
	// x and y are used as variable names for brevity, in a more formal
	// implementation, you should probably change the names

	RbNode* y;

	// maintain red-black tree properties after adding x
	while (x != tree->root && x->parent->color == RED)
	{
		// Parent node is .Colored red; 
		if (x->parent == x->parent->parent->left)   // determine traversal path			
		{                                       // is it on the Left or Right subtree?
			y = x->parent->parent->right;          // get uncle
			if (y != NULL && y->color == RED)
			{   // uncle is red; change x's Parent and uncle to black
				x->parent->color = BLACK;
				y->color = BLACK;
				// grandparent must be red. Why? Every red node that is not 
				// a leaf has only black children 
				x->parent->parent->color = RED;
				x = x->parent->parent;    // continue loop with grandparent
			}
			else
			{
				// uncle is black; determine if x is greater than Parent
				if (x == x->parent->right)
				{   // yes, x is greater than Parent; rotate Left
					// make x a Left child
					x = x->parent;
					rotateLeft(tree, x);
				}
				// no, x is less than Parent
				x->parent->color = BLACK;    // make Parent black
				x->parent->parent->color = RED;       // make grandparent black
				rotateRight(tree, x->parent->parent);                   // rotate right
			}
		}
		else
		{   // x's Parent is on the Right subtree
			// this code is the same as above with "Left" and "Right" swapped
			y = x->parent->parent->left;
			if (y != NULL && y->color == RED)
			{
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
			}
			else
			{
				if (x == x->parent->left)
				{
					x = x->parent;
					rotateRight(tree, x);
				}
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rotateLeft(tree, x->parent->parent);
			}
		}
	}
	tree->root->color = BLACK;      // rbTree should always be black
}

///<summary>
/// restoreAfterDelete
/// Deletions from red-black trees may destroy the red-black 
/// properties. Examine the tree and restore. Rotations are normally 
/// required to restore it
///</summary>
static void restoreAfterDelete(RbTree* tree, RbNode* x)
{
	// maintain Red-Black tree balance after deleting node 			

	RbNode* y;

	while (x != tree->root && x->color == BLACK)
	{
		if (x == x->parent->left)         // determine sub tree from parent
		{
			y = x->parent->right;         // y is x's sibling 
			if (y->color == RED)
			{   // x is black, y is red - make both black and rotate
				y->color = BLACK;
				x->parent->color = RED;
				rotateLeft(tree, x->parent);
				y = x->parent->right;
			}
			if (y->left->color == BLACK &&
				y->right->color == BLACK)
			{   // children are both black
				y->color = RED;     // change parent to red
				x = x->parent;                   // move up the tree
			}
			else
			{
				if (y->right->color == BLACK)
				{
					y->left->color = BLACK;
					y->color = RED;
					rotateRight(tree, y);
					y = x->parent->right;
				}
				y->color = x->parent->color;
				x->parent->color = BLACK;
				y->right->color = BLACK;
				rotateLeft(tree, x->parent);
				x = tree->root;
			}
		}
		else
		{   // right subtree - same as code above with right and left swapped
			y = x->parent->left;
			if (y->color == RED)
			{
				y->color = BLACK;
				x->parent->color = RED;
				rotateRight(tree, x->parent);
				y = x->parent->left;
			}
			if (y->right->color == BLACK &&
				y->left->color == BLACK)
			{
				y->color = RED;
				x = x->parent;
			}
			else
			{
				if (y->left->color == BLACK)
				{
					y->right->color = BLACK;
					y->color = RED;
					rotateLeft(tree, y);
					y = x->parent->left;
				}
				y->color = x->parent->color;
				x->parent->color = BLACK;
				y->left->color = BLACK;
				rotateRight(tree, x->parent);
				x = tree->root;
			}
		}
	}
	x->color = BLACK;
}

///<summary>
/// Delete
/// Delete a node from the tree and restore red black properties
///<summary>
static void deleteRbTreeItemImpl(RbTree* tree, RbNode* z)
{
	// A node to be deleted will be: 
	//		1. a leaf with no children
	//		2. have one child
	//		3. have two children
	// If the deleted node is red, the red black properties still hold.
	// If the deleted node is black, the tree needs rebalancing

	RbNode* x; // work node to contain the replacement node
	RbNode* y; // work node 

	// find the replacement node (the successor to x) - the node one with 
	// at *most* one child. 
	if (z->left == SENTINEL || z->right == SENTINEL)
		y = z;                      // node has sentinel as a child
	else
	{
		// z has two children, find replacement node which will 
		// be the leftmost node greater than z
		y = z->right;                        // traverse right subtree	
		while (y->left != SENTINEL)      // to find next node in sequence
			y = y->left;
	}

	// at this point, y contains the replacement node. it's content will be copied 
	// to the valules in the node to be deleted

	// x (y's only child) is the node that will be linked to y's old parent. 
	if (y->left != SENTINEL)
		x = y->left;
	else
		x = y->right;

	// replace x's parent with y's parent and
	// link x to proper subtree in parent
	// this removes y from the chain
	x->parent = y->parent;
	if (y->parent != NULL && y->parent != SENTINEL)
		if (y == y->parent->left)
			y->parent->left = x;
		else
			y->parent->right = x;
	else
		tree->root = x;         // make x the root node

	// copy the values from y (the replacement node) to the node being deleted.
	// note: this effectively deletes the node. 
	if (y != z)
	{
		if (tree->itemDtor != NULL)
		{
			tree->itemDtor(z->key, z->data);
		}

		z->key = y->key;
		z->data = y->data;
		free(y);
		y = z;
	}

	if (y->color == BLACK)
		restoreAfterDelete(tree, x);
}

static void releaseRbTreeImpl(RbTree* tree, RbNode* node)
{
	if (node != SENTINEL)
	{
		releaseRbTreeImpl(tree, node->left);
		releaseRbTreeImpl(tree, node->right);

		if (tree->itemDtor != NULL) 
		{
			tree->itemDtor(node->key, node->data);
		}

		free(node);
	}
}

RbTree* createRbTree(intFunc2 keyComparer, action2 itemDtor, action2 defaultItemPrinter)
{
	RbTree* tree = myAlloc(RbTree);
	tree->count = 0;
	tree->keyComparer = keyComparer;
	tree->itemDtor = itemDtor;
	tree->defaultItemPrinter = defaultItemPrinter;
	tree->root = SENTINEL;
	return tree;
}

//static int compareStrings(char* a, char* b)
//{
//	int rc = strcmp(a, b);
//	printf("Comparing '%s' against '%s' resulted with %d\n", a, b, rc);
//	return rc;
//}

static void stringKeyPrinter(char* k, void* v)
{
	UNUSED(v);
	printf("%s", k);
}

RbTree* createRbTreeByString()
{
	return createRbTree((intFunc2)strcmp, NULL, (action2)stringKeyPrinter);
}

int getRbTreeCount(RbTree* tree)
{
	return tree->count;
}

void* findRbTreeItemOrAdd(RbTree* tree, void* key, void* data)
{
	RbFindResult place = findRbTreeItemImpl(tree, key);
	void* existingData;

	if (place.node != SENTINEL)
	{
		existingData = place.node->data;
	}
	else
	{
		existingData = NULL;

		RbNode* node = myAlloc(RbNode);
		node->color = RED;
		node->key = key;
		node->data = data;
		node->left = SENTINEL;
		node->right = SENTINEL;
		node->parent = place.prev;

		if (place.prev != SENTINEL)
		{
			if (tree->keyComparer(place.prev->key, key) < 0)
			{
				place.prev->right = node;
			}
			else
			{
				place.prev->left = node;
			}
		}
		else
		{
			tree->root = node;
		}

		restoreAfterInsert(tree, node);

		tree->count++;
	}

	return existingData;
}

bool tryAddRbTreeItem(RbTree* tree, void* key, void* data)
{
	return findRbTreeItemOrAdd(tree, key, data) == NULL;
}

bool tryRemoveRbTreeItem(RbTree* tree, void* key)
{
	RbFindResult place = findRbTreeItemImpl(tree, key);
	if (place.node != SENTINEL)
	{
		deleteRbTreeItemImpl(tree, place.node);

		tree->count--;

		return true;
	}
	else
	{
		return false;
	}
}

void* findRbTreeItem(RbTree* tree, void* key)
{
	RbFindResult place = findRbTreeItemImpl(tree, key);
	return place.node != SENTINEL ? place.node->data : NULL;
}

void releaseRbTree(RbTree* tree)
{
	releaseRbTreeImpl(tree, tree->root);
	free(tree);
}

static void traceRbTreeImpl(RbNode* node, int depth, action2 itemHandler)
{
	if (node != SENTINEL)
	{
		traceRbTreeImpl(node->left, depth + 1, itemHandler);
		for (int i = 0; i < depth; i++) printf("  ");
		itemHandler(node->key, node->data);
		printf("\n");
		traceRbTreeImpl(node->right, depth + 1, itemHandler);
	}
}

void traceRbTree(RbTree* tree)
{
	printf("\n{\n");
	traceRbTreeImpl(tree->root, 0, tree->defaultItemPrinter);
	printf("}\n");
}

static void traverseRbTreeImpl(RbNode* node, action3 itemHandler, void* state)
{
	if (node && node != SENTINEL)
	{
		traverseRbTreeImpl(node->left, itemHandler, state);
		itemHandler(node->key, node->data, state);
		traverseRbTreeImpl(node->right, itemHandler, state);
	}
}

void traverseRbTree(RbTree* tree, action3 act, void* state)
{
	traverseRbTreeImpl(tree->root, act, state);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

LinkedList* createLinkedList()
{
	LinkedList* list = myAlloc(LinkedList);
	list->count = 0;
	list->first = NULL;
	list->last = NULL;
	return list;
}

void addFirstLinkedListItem(LinkedList* list, void* data)
{
	LinkedListItem* item = myAlloc(LinkedListItem);
	item->data = data;
	item->next = list->first;
	list->first = item;

	if (list->count <= 0)
	{
		list->last = item;
	}

	list->count++;
}

void addLastLinkedListItem(LinkedList* list, void* data)
{
	LinkedListItem* item = myAlloc(LinkedListItem);
	item->data = data;
	item->next = NULL;

	if (list->count > 0)
	{
		list->last->next = item;
		list->last = item;
	}
	else
	{
		list->first = item;
		list->last = item;
	}

	list->count++;
}

void traverseLinkedList(LinkedList* list, action2 act, void* state)
{
	for (LinkedListItem* item = list->first; item != NULL; item = item->next)
	{
		act(item->data, state);
	}
}

void traverseAndRemove(LinkedList* list, boolFunc2 condition, void* state)
{
	LinkedListItem* prev = NULL;
	LinkedListItem* item = list->first;

	while (item != NULL)
	{
		LinkedListItem* next = item->next;

		if (condition(item->data, state))
		{
			if (prev == NULL)
				list->first = next;
			
			if (next == NULL)
				list->last = prev;

			if (prev != NULL)
				prev->next = next;

			free(item);
			list->count--;
		}
		else
		{
			prev = item;
		}
	
		item = next;
	}
}

void releaseLinkedList(LinkedList* list)
{
	for (LinkedListItem *item = list->first, *next = NULL; item != NULL; item = next)
	{
		next = item->next;
		free(item);
	}

	free(list);
}

static char* concatListOfStringsImpl(LinkedListItem* item, int length)
{
	char* result;
	if (item == NULL)
	{
		result = myAllocArray(char, length + 1);
		result[length] = 0;
		result += length;
	}
	else
	{
		char* str = (char*)item->data;
		int len = (int)strlen(str);
		char* buffer = concatListOfStringsImpl(item->next, length + len);
		result = buffer - len;
		memcpy(result, str, len);
	}
	return result;
}

char* concatListOfStrings(LinkedList* list)
{
	return concatListOfStringsImpl(list->first, 0);
}

