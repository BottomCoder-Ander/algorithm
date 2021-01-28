/*
 * 
 */

#ifndef _BTREE_H
#define _BTREE_H

#include <cstddef>
#include <cassert>
#include <cstdio>
#include <cstring>

namespace btree{

template<typename KeyType> 
class BTree{
  struct BTNode{
    int keynum;                     //结点关键字个数
    KeyType *key;        //关键字数组，key[0]不使用 
    struct BTNode *parent;            //双亲结点指针
    struct BTNode **ptr;         //孩子结点指针数组 
    BTNode(int m) {
      keynum = 0;
      key = new KeyType[m + 1];
      ptr = new BTNode*[m + 1];
      memset(ptr, 0, (m + 1) * sizeof(BTNode*));
      parent = NULL;
    }
    ~BTNode(){
      delete []key;
      delete []ptr;
    }
  } ;
public:
  BTree(int m): m(m), 
                root(NULL), 
                max_keynum(m - 1), 
                min_keynum((m - 1) >> 1){
    
    if(m < 4){
      fprintf(stderr, "m >= 4 required!!!\r\n");
      exit(-1);
    }
  }

  bool search(KeyType &key);
  bool insert(KeyType key);
  void del(KeyType key);
  void traverse();

  ~BTree(){
      _destroyBTree(root);
  }

private:
  bool _searchNode(BTNode *p, KeyType key, int &idx) const;
  bool _searchBTree(KeyType key, BTNode *&p, int &idx) const;
  void _insertBTNode(BTNode *&p, int idx, KeyType key, BTNode *q);
  void _splitBTNode(BTNode *p, BTNode *&q);
  void _newRoot(KeyType key,BTNode *p,BTNode *q);
  void _insertBTree(BTNode *p, int idx, KeyType key);
  void _substitution(BTNode *p, int idx);
  void _moveRight(BTNode *p, int idx);
  void _moveLeft(BTNode *p, int idx);
  void _combine(BTNode *p, int idx);
  void _adjustBTree(BTNode *p, int idx);
  bool _btNodeDelete(BTNode *p, KeyType key);
  void _destroyBTree(BTNode* &p);

private:
  
  int m;
  int max_keynum, min_keynum;
  BTNode* root;
};

} //namespace btree

#endif 