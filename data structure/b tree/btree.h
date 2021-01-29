/*
 * 
 */

#ifndef _BTREE_H
#define _BTREE_H

#include <cstddef>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdint.h>

#define DEBUG

namespace btree{

template<typename KeyType> 
class BTree{
  struct BTNode{
    size_t keynum;                     //结点关键字个数
    KeyType *key;        //关键字数组，key[0]不使用 
    struct BTNode *parent;            //双亲结点指针
    struct BTNode **ptr;         //孩子结点指针数组 
    BTNode(uint32_t m) {
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
  BTree(uint32_t m): m(m), 
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
  bool _searchNode(BTNode *p, KeyType key, size_t &idx) const;
  bool _searchBTree(KeyType key, BTNode *&p, size_t &idx) const;
  void _insertBTNode(BTNode *&p, size_t idx, KeyType key, BTNode *q);
  void _splitBTNode(BTNode *p, BTNode *&q);
  void _newRoot(KeyType key,BTNode *p,BTNode *q);
  void _insertBTree(BTNode *p, size_t idx, KeyType key);
  void _substitution(BTNode *p, size_t idx);
  void _moveRight(BTNode *p, size_t idx);
  void _moveLeft(BTNode *p, size_t idx);
  void _combine(BTNode *p, size_t idx);
  void _adjustBTree(BTNode *p, size_t idx);
  bool _btNodeDelete(BTNode *p, KeyType key);
  void _destroyBTree(BTNode* &p);

private:
  
  uint32_t m;
  uint32_t max_keynum, min_keynum;
  BTNode* root;
};

} //namespace btree

#endif 