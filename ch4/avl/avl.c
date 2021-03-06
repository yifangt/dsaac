/*
 * avl.c
 *
 *  Created on: 2012-8-2
 *      Author: mcxiaoke
 */

#include "avl.h"
#include "utils.h"
#include <time.h>

/*******************************************************************************************
 *
 * 二叉查找树要求一个树的根节点必然大于其左子树中的所有节点, 同时必然小于其右子树中的所有节点
 * 也就是说,如果按照中序遍历二叉查找树, 它必然是严格递增的
 *
 * AVL树定义：每个节点的左子树和右子树的高度最多差1的二叉查找树
 *
 * 1. 概述
 AVL树是最早提出的自平衡二叉树，在AVL树中任何节点的两个子树的高度最大差别为一，所以它也被称为高度平衡树
 AVL树得名于它的发明者G.M. Adelson-Velsky和E.M. Landis
 AVL树种查找、插入和删除在平均和最坏情况下都是O（log n），增加和删除可能需要通过一次或多次树旋转来重新平衡这个树
 本文介绍了AVL树的设计思想和基本操作
 2. 基本术语
 有四种种情况可能导致二叉查找树不平衡，分别为：
 （1）LL：插入一个新节点到根节点的左子树（Left）的左子树（Left），导致根节点的平衡因子由1变为2
 （2）RR：插入一个新节点到根节点的右子树（Right）的右子树（Right），导致根节点的平衡因子由-1变为-2
 （3）LR：插入一个新节点到根节点的左子树（Left）的右子树（Right），导致根节点的平衡因子由1变为2
 （4）RL：插入一个新节点到根节点的右子树（Right）的左子树（Left），导致根节点的平衡因子由-1变为-2

 5			 3					5			   3
 /			  \				   /				\
	  4					4			  3				 	 5
 /					 \				\				/
 3					  5				 4			   4

 调整后都为：
 4
 /    \
		   3		5
 针对四种种情况可能导致的不平衡，可以通过旋转使之变平衡。有两种基本的旋转：
 （1）左旋转：将根节点旋转到（根节点的）右孩子的左孩子位置
 （2）右旋转：将根节点旋转到（根节点的）左孩子的右孩子位置
 3. AVL树的旋转操作
 AVL树的基本操作是旋转，有四种旋转方式，分别为：
 左旋转[顺时针]，右旋转[逆时针]，左右旋转（先左后右），右左旋转（先右后左），
 实际上，这四种旋转操作两两对称，因而也可以说成两类旋转操作

 *  关于旋转可以参考维基百科的一幅图，很好懂
 *  http://zh.wikipedia.org/zh/AVL树
 *  http://cnn237111.blog.51cto.com/2359144/846440
 *  http://www.cnblogs.com/heqile/archive/2011/11/28/2266385.html
 *  http://www.cnblogs.com/heqile/archive/2011/11/28/2265713.html
 *  http://www.asiteof.me/2010/06/avl/
 *
 *  C++实现
 *  http://www.cppblog.com/Darren/archive/2009/05/07/82190.html
 *
 *  删除
 *  http://hi.baidu.com/shihao_dong/item/d2ff51164a7d4324f6625ca5
 *
 *  二叉查找树
 *  http://www.cppblog.com/guogangj/archive/2009/10/26/99502.html
 *
 *  其它
 *  http://oopweb.com/Algorithms/Documents/AvlTrees/Volume/AvlTrees.htm
 *
 *
 *******************************************************************************************/
// 树的高度
static int height(Position p) {
	if (p == NULL ) {
		return -1;
	} else {
		return p->height;
	}
}

// 显示节点值，辅助方法
static void show(AvlTree t) {
	if (t != NULL ) {
		printf("%d ", t->element);
//		printf("%d(%d) ", t->element, height(t));
	} else {
//		printf("NULL ");
	}
}

// 创建节点，辅助方法
static AvlNode* makeNode(int x) {
	AvlNode* node = (AvlNode*) malloc(sizeof(AvlNode));
	if (node != NULL ) {
		node->element = x;
		node->height = 0;
		node->left = NULL;
		node->right = NULL;
		//printf("makeNode: %d [0x%x]\n", value, node);
	}
	return node;
}

/**
 * 旋转之前:
 * 						[X5]
 * 				[Y4]
 * 		[Z3]
 * 旋转之后：
 * 					[Y4]
 * 			[Z3]			[X5]
 *
 */
// LL单旋转，以中间节点为中心顺时针旋转
// 左--右 单旋， 参考图：4-32 p82
// x围绕y顺时针旋转
static Position singleRotateWithLeft(Position x) {
	if (x->left == NULL ) {
		return x;
	}
	loginfo("singleRotateWithLeft() %d(%d)\n", x->element, height(x));
	Position y;
	y = x->left;
	x->left = y->right; // y->right可能为空
	y->right = x; // y->right指向x，y->left 保持不变
	// x成为y的右子树，它的高度等于原来左右子树高度较大者+1
	x->height = max(height(x->left), height(x->right)) + 1;
	// y成为新子树的根，它的高度等于左右子树高度较大者+1
	y->height = max(height(y->left), height(y->right)) + 1;
	return y; // y作为新子树的根节点返回
}

/**
 * 旋转之前:
 * 		[X3]
 * 				[Y4]
 * 						[Z5]
 * 旋转之后：
 * 					[Y4]
 * 			[X3]			[Z5]
 *
 */
// RR单旋转，以中间节点为中心逆时针旋转
// 右--左 单旋， 参考图：4-32 p83
// x围绕y逆时针旋转
static Position singleRotateWithRight(Position x) {
	if (x->right == NULL ) {
		return x;
	}
	loginfo("singleRotateWithRight() %d(%d)\n", x->element, height(x));
	Position y;
	y = x->right;
	x->right = y->left; // y->left可能为空
	y->left = x; // y->left 指向x，y->right 保持不变
	// x成为y的左子树，它的高度等于原来左右子树高度较大者+1
	x->height = max(height(x->right), height(x->left)) + 1;
	// y成为新子树的根，它的高度等于左右子树高度较大者+1
	y->height = max(height(y->right), height(y->left)) + 1;
	return y; // y作为新子树的根节点返回
}

/**
 * 旋转之前:
 * 						[X5]
 * 				[Y3]
 * 						[Z4]
 * 								[ - ]
 * 第一次旋转：z-y左旋，逆时针，右-左单旋
 * 相当于以Y为根节点，旋转调整Y-Z-[ ]子树
 * 						[X5]
 * 				[Z4]
 * 		[Y3]
 *
 * 	第二次旋转：右旋，顺时针，左-右单旋
 * 	相当于以X为根节点，旋转调整X-Z-Y子树
 * 					[Z4]
 * 			[Y3]			[X5]
 *
 */
// LR左右双旋
// 先进行左旋，再进行右旋
// 第一步：z围绕y逆时针旋转
// 第二步：x围绕z顺时针旋转
static Position doubleRotateWithLeft(Position x) {
	// x->left is y
	if (x->left == NULL || x->left->right == NULL ) {
		return x;
	}
	loginfo("doubleRotateWithLeft() %d(%d)\n", x->element, height(x));
	x->left = singleRotateWithRight(x->left);
	return singleRotateWithLeft(x);
}

/**
 * 旋转之前:
 * 				[X3]
 * 						[Y5]
 * 				[Z4]
 *		[ - ]
 *
 * 第一次旋转:z-y右旋，顺时针，左-右单旋
 * 相当于以Y为根节点，旋转调整Y-Z-[ ]子树
 * 		[X3]
 * 				[Z4]
 * 						[Y5]
 * 第二次旋转：左旋，逆时针，右-左单旋
 * 相当于以X为根节点，旋转调整X-Z-Y子树
 * 					[Z4]
 * 			[X3]			[Y5]
 *
 */
// RL右左旋转
// 先进行右旋，再进行左旋
// 第一步：z围绕y顺时针旋转
// 第二步：x围绕z逆时针旋转
static Position doubleRotateWithRight(Position x) {
	// x->right is y
	if (x->right == NULL || x->right->left == NULL ) {
		return x;
	}
	loginfo("doubleRotateWithRight() %d(%d)\n", x->element, height(x));
	x->right = singleRotateWithLeft(x->right);
	return singleRotateWithRight(x);
}

static void random(int data[], int n) {
	srand((unsigned) time(NULL ));
//	srand(9);
	for (int i = 0; i < n; i++) {
		data[i] = i;
	}
	for (int i = 0; i < n; i++) {
		int j = rand() % n;
		int tmp = data[i];
		data[i] = data[j];
		data[j] = tmp;
	}

	printf("INPUT DATA:\n");
	for(int i=0; i< 10; ++i){
		printf("%d ", data[i]);
	}
	printf("\n");
}

// 平衡调整
static void balance(AvlTree *t) {
	AvlTree at = *t;
	if (at == NULL ) {
		return;
	}
	if ((height(at->left) - height(at->right)) == 2) {
		AvlTree al = at->left;
		if (height(al->left) > height(al->right)) {
			*t = singleRotateWithLeft(*t); // LL
		} else {
			*t = doubleRotateWithLeft(*t); // LR
		}
	} else if ((height(at->right) - height(at->left)) == 2) {
		AvlTree ar = at->right;
		if (height(ar->right) > height(ar->left)) {
			*t = singleRotateWithRight(*t); // RR
		} else {
			*t = doubleRotateWithRight(*t); // RL
		}
	}
}

AvlTree avl_create(AvlTree* pt) {
	int data[100];
	random(data, 100);
	for (int i = 0; i < DATA_MAX; ++i) {
		int x = data[i];
		*pt = avl_insert(x, *pt);
	}
	loginfo("avl_create() TreeRoot: %d<%p>", (*pt)->element, *pt);
	return *pt;
}

// 插入节点，调整保持平衡
Position avl_insert(int x, AvlTree t) {
	// 如果树为空，插入根节点
	if (t == NULL ) {
		t = makeNode(x);
//		printf("Insert %d\t<%p>\n", t->element, t);
//		(*t)->height=1;
//		return t;
	}
	// 如果小于当前节点，插入到左子树
	else if (x < t->element) {
		t->left = avl_insert(x, t->left);
//		balance(t);
		// 不平衡，进行旋转
		if ((height(t->left) - height(t->right)) == 2) {
//			AvlTree tl = t->left;
//			AvlTree tr = t->right;
//			printf("[Left Balance] T:%d(%d) TL:%d(%d) TR:%d(%d)\n", t->element,
//					t->height, tl->element, tl->height,
//					tr == NULL ? -1 : tr->element, height(tr));
			if (x < t->left->element) {
				t = singleRotateWithLeft(t); // LL
			} else {
				t = doubleRotateWithLeft(t); // LR
			}
		}
	}
	// 如果大于当前节点，插入到右子树
	else if (x > t->element) {
		t->right = avl_insert(x, t->right);
//		balance(t);
		//不平衡，进行旋转
		if ((height(t->right) - height(t->left)) == 2) {
//			AvlTree tl = t->left;
//			AvlTree tr = t->right;
//			printf("[Right Balance] T:%d(%d) TL:%d(%d) TR:%d(%d)\n", t->element,
//					t->height, tl == NULL ? -1 : tl->element, height(tl),
//					tr->element, tr->height);
			if (x > t->right->element) {
				t = singleRotateWithRight(t); // RR
			} else {
				t = doubleRotateWithRight(t); // RL
			}
		}
	}

	t->height = max(height(t->left), height(t->right)) + 1;
//	printf("avl_insert() %d<%p> (%d)\n", t->element, t, t->height);
	return t;
}

AvlTree avl_delete(int x, AvlTree t) {
	//直接删除需要很多调整，可以用惰性删除
	if (t == NULL ) {
		return NULL ;
	}
	if (x < t->element) {
		t->left = avl_delete(x, t->left);
		t->height = max(height(t->left), height(t->right)) + 1;
		// 右子树不平衡
		if ((height(t->right) - height(t->left)) == 2) {
			Position pr = t->right;
			if (height(pr->right) > height(pr->left)) {
				// RR
				t->right = singleRotateWithRight(t->right);
			} else {
				// RL
				t->right = doubleRotateWithRight(t->right);
			}
		}
	} else if (x > t->element) {
		t->right = avl_delete(x, t->right);
		t->height = max(height(t->left), height(t->right)) + 1;
		// 左子树不平衡
		if (height(t->left) - height(t->left) == 2) {
			Position pl = t->left;
			if (height(pl->left) > height(pl->right)) {
				// LL
				t->left = singleRotateWithLeft(t->left);
			} else {
				// LR
				t->left = doubleRotateWithLeft(t->left);
			}
		}
	} else { // if x == t->element, node found.
		if (t->right == NULL ) {
			// 没有右子树的话，直接用T的左子树节点取代T
			AvlTree node = t;
			t = t->left;
			free(node);
		} else {
			AvlTree node = avl_findMin(t->right);
			t->element = node->element;
			t->right = avl_delete(t->element, t->right);
			t->height = max(height(t->left), height(t->right)) + 1;
			return t;
		}
	}
	t->height = max(height(t->left), height(t->right)) + 1;

	return t;
}

AvlTree rotate(AvlTree t) {
//    if(Height(t->lchild) - Height(t->rchild) == 2){
//        if(Height(t->lchild->lchild) >= Height(T->lchild->rchild)){
//            T = SingleRotateWithLeft(T);
//        }
//        else{
//            T = DoubleRotateWithLeft(T);
//        }
//    }
//    if(Height(T->rchild) - Height(T->lchild) ==2){
//        if(Height(T->rchild->rchild) >= Height(T->rchild->lchild)){
//            T = SingleRotateWithRight(T);
//        }
//        else{
//            T = DoubleRotateWithRight(T);
//        }
//    }
//    return T;
	return t;
}

// 清空AVL树的节点
AvlTree avl_clear(AvlTree t) {
	if (t != NULL ) {
		avl_clear(t->left);
		avl_clear(t->right);
		free(t);
	}
	return t;
}

// 查找，非递归版
Position avl_find(int x, AvlTree t) {
	while (t != NULL && x != t->element) {
		if (x < t->element) {
			t = t->left;
		} else {
			t = t->right;
		}
	}
	return t;
}

// 查找最大值节点
Position avl_findMin(AvlTree t) {
	if (t != NULL ) {
		while (t->left != NULL ) {
			t = t->left;
		}
	}
	return t;
}

// 查找最小值节点
Position avl_findMax(AvlTree t) {
	if (t == NULL ) {
		return NULL ;
	}
	if (t->right == NULL ) {
		return t;
	} else {
		return avl_findMax(t->right);
	}
}

// 中序遍历，递归版
void avl_inOrder(AvlTree t) {
	if (t != NULL ) {
		avl_inOrder(t->left);
		show(t);
		avl_inOrder(t->right);
	}
}

void avl_preOrder(AvlTree t){
	if (t != NULL ) {
		show(t);
		avl_preOrder(t->left);
		avl_preOrder(t->right);
	}
}

void test(void){
//	setvbuf(stdout, 0, _IONBF, 0);
//	srand(0);
	AvlTree t = NULL;
	avl_create(&t);
	printf("\nOUTPUT DATA:\n");
	avl_inOrder(t);
//	avl_preOrder(t);
}
