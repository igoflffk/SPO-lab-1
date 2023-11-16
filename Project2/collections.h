
#pragma once

#include "util.h"

#if 1 // region RbTree {

/// <summary>
/// The color of node in a red-black tree
/// </summary>
enum RbColor {
	BLACK = 0,
	RED = 1,
};

/// <summary>
/// The node of a red-black tree
/// </summary>
typedef struct RbNode {
	struct RbNode* parent;
	struct RbNode* left;
	struct RbNode* right;
	int color;
	void* key;
	void* data;
} RbNode;

/// <summary>
/// The red-black tree instance with its accompanying data-specific handlers
/// </summary>
typedef struct RbTree {
	RbNode* root;
	int count;
	intFunc2 keyComparer;
	action2 itemDtor;
	action2 defaultItemPrinter;
} RbTree;

/// <summary>
/// Create a new red-black tree instance
/// </summary>
/// <param name="keyComparer">function used to compare keys during red-black tree lookup operations</param>
/// <param name="itemDtor">function called for each item in a tree during its destruction</param>
/// <param name="defaultItemPrinter">function call for ecah item in a tree during its debug tracing</param>
/// <returns></returns>
RbTree* createRbTree(intFunc2 keyComparer, action2 itemDtor, action2 defaultItemPrinter);

/// <summary>
/// Create a new red-black tree specified for string keys of char* type
/// </summary>
RbTree* createRbTreeByString(void);

/// <summary>
/// Red-black tree items counter accessor
/// </summary>
/// <param name="tree"></param>
/// <returns></returns>
int getRbTreeCount(RbTree* tree); // TODO consider using opaq types for collections

/// <summary>
/// Add a new item to the red-black tree if the key doesn't exists yet
/// </summary>
/// <returns>True if the new item was introduced</returns>
bool tryAddRbTreeItem(RbTree* tree, void* key, void* data);

/// <summary>
/// Find an existing item in the red-black tree by the given key, add a new item if the key was not found.
/// </summary>
/// <returns>Existing data reference on NULL of new item was introduced</returns>
void* findRbTreeItemOrAdd(RbTree* tree, void* key, void* data);

/// <summary>
/// Find an existing item in the red-black tree and remote it if exists
/// </summary>
/// <returns>True is item was found and removed, false otherwise</returns>
bool tryRemoveRbTreeItem(RbTree* tree, void* key);

/// <summary>
/// Find an existing item in the red-black tree by the given key
/// </summary>
/// <returns>NULL if item doesn't exists or a corresponding data reference</returns>
void* findRbTreeItem(RbTree* tree, void* key);

/// <summary>
/// Release all the memory allocations associated with the red-black tree
/// </summary>
/// <param name="tree"></param>
void releaseRbTree(RbTree* tree);

/// <summary>
/// Trace the AST tree to standard output using its item-printing function
/// </summary>
/// <param name="tree"></param>
void traceRbTree(RbTree* tree);

/// <summary>
/// Call action for each key-value item
/// </summary>
/// <param name="list"></param>
/// <param name="act"></param>
/// <param name="state"></param>
void traverseRbTree(RbTree* tree, action3 act, void* state);

#endif // } region RbTree

#if 1 // region LinkedList {


typedef struct LinkedListItem {
	struct LinkedListItem* next;
	void* data;
} LinkedListItem;

typedef struct LinkedList {
	LinkedListItem* first;
	LinkedListItem* last;
	int count;
} LinkedList;

LinkedList* createLinkedList(void);
void addFirstLinkedListItem(LinkedList* list, void* data);
void addLastLinkedListItem(LinkedList* list, void* data);
void traverseLinkedList(LinkedList* list, action2 act, void* state);
void traverseAndRemove(LinkedList* list, boolFunc2 condition, void* state);
void releaseLinkedList(LinkedList* list);

char* concatListOfStrings(LinkedList* list);

#endif // } region LinkedList

