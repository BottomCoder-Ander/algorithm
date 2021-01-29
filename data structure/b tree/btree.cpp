#include"btree.h"

#include <cstdio>
#include <malloc.h>
#include <cstring>
#include <cstdlib> 
#include <queue>

namespace btree{

#ifdef DEBUG
#define checkpar(p) do { \
    if(p != NULL && p->ptr[0] != NULL) {\
        for(size_t i = 0; i <= p->keynum; ++i){\
            assert(p->ptr[i]->parent == p);\
        }\
    }\
}while(0)
#endif

/* 
 * 在结点p中查找关键字k的插入位置i
 */
template<typename KeyType>
bool BTree<KeyType>::_searchNode(BTree<KeyType>::BTNode *p, KeyType key, size_t &idx) const {

    if(p->keynum < 16) {
        for(idx = 0; idx < p->keynum && p->key[idx+1] <= key; ++idx);
        return idx > 0 && p->key[idx] == key;
    }
    //二分，找到第一个大于的，upper_bound。
    size_t left = 1, right = p->keynum;
    while(left <= right){
        size_t mid = (left + right) >> 1;
        if(p->key[mid] <= key){
            left = mid + 1;       
        }else{
            right = mid - 1;
        }
    }

    if(left > p->keynum || p->key[left] > key) --left;
    idx = left;
    return idx > 0 && p->key[idx] == key;
}

/*
 * 在树t上查找关键字k, 返回结果(pt, idx, sucess)。
 * 若查找成功,则特征值sucess = 1, 关键字key是指针pt所指结点中第idx个关键字；
 * 否则特征值sucess = 0, 关键字k的插入位置为pt结点的第idx个
 */
template<typename KeyType>
bool BTree<KeyType>::_searchBTree(KeyType key, BTree<KeyType>::BTNode *&p, size_t &idx) const {
    BTNode *p_par = NULL;                         //初始化结点p和结点q,p指向待查结点,q指向p的双亲               
    p = root;
    while(p != NULL){
        if(_searchNode(p, key, idx)) {
            return true;
        } else{                                       //查找失败 
            p_par = p;                            
            p = p->ptr[idx];
        }
    }

    p = p_par;
    return false;                                     //查找失败
}

template<typename KeyType>
bool BTree<KeyType>::search(KeyType &key) {
    
    BTNode *p; 
    size_t idx;
    if(_searchBTree(key, p, idx)) {
        key = p->key[idx];
        return true;
    }
    return false;
}

/*
 * 将关键字key和结点q分别插入到p->key[idx+1] 和 p->ptr[idx+1]中
 */

template<typename KeyType>
void BTree<KeyType>::_insertBTNode(BTree<KeyType>::BTNode *&p, size_t idx, KeyType key, BTNode *q) {
    memmove(&(p->key[idx + 2]), &(p->key[idx + 1]), (p->keynum - idx) * sizeof(KeyType));
    memmove(&(p->ptr[idx + 2]), &(p->ptr[idx + 1]), (p->keynum - idx) * sizeof(BTNode *)); 
    p->key[idx + 1] = key;
    p->ptr[idx + 1] = q;
    if(q != NULL) 
        q->parent = p;
    p->keynum++;
}


template<typename KeyType>
void BTree<KeyType>::_splitBTNode(BTree<KeyType>::BTNode *p, BTree<KeyType>::BTNode *&q) {
//将结点p分裂成两个结点,前一半保留, 后一半移入结点q

    size_t s = (m + 1) >> 1;
    q = new BTNode(m);             //给结点q分配空间

    q->ptr[0] = p->ptr[s];                            //后一半移入结点q
    memmove(&(q->key[1]), &(p->key[s + 1]), (m - s) * sizeof(KeyType));
    memmove(q->ptr, &(p->ptr[s]), (m - s + 1) * sizeof(BTNode *));
    

    q->keynum = m - s;                
    q->parent = p->parent;

    for(size_t i = 0; i <= m - s; ++i)                      //修改双亲指针 
        if(q->ptr[i] != NULL) 
            q->ptr[i]->parent = q;

    p->keynum = s - 1;                                  //结点p的前一半保留,修改结点p的keynum
}


template<typename KeyType>
void BTree<KeyType>::_newRoot(KeyType key, BTree<KeyType>::BTNode *p, BTree<KeyType>::BTNode *q) {
//生成新的根结点t,原p和q为子树指针
    root = new BTNode(m);             //分配空间 
    root->keynum = 1;
    root->ptr[0] = p;
    root->ptr[1] = q;
    root->key[1] = key;
    if(p != NULL)                                     //调整结点p和结点q的双亲指针 
        p->parent = root;
    if(q != NULL) 
        q->parent = root;
    root->parent = NULL;
}


/* 确保关键字key不存在。
 * 在树t上结点q的key[idx]与key[idx+1]之间插入关键字k。若引起
 * 结点过大,则沿双亲链进行必要的结点分裂调整 */
template<typename KeyType>
void BTree<KeyType>::_insertBTree(BTree<KeyType>::BTNode *p, size_t idx, KeyType key) {
    BTNode *q = NULL;
    size_t  s;                   //设定需要新结点标志和插入完成标志 
    if(p == NULL)                                     //t是空树
        _newRoot(key, NULL, NULL);                     //生成仅含关键字k的根结点t
    else{
        KeyType x = key;
        while(1){
            _insertBTNode(p, idx, x, q);                  //将关键字x和结点q分别插入到p->key[i+1]和p->ptr[i+1]
            if (p->keynum <= max_keynum) 
                return; 
            _splitBTNode(p, q);                   //分裂结点 
            
            x = p->key[(m + 1) >> 1];
            if(p->parent){                      //p不是根，查找x的插入位置
                p = p->parent;
                _searchNode(p, x, idx);
            }else{                                //p是根，需要建一个根，保存关键字x, p,q为两个儿子 
                _newRoot(x, p, q);
                return;
            }
            
        }
    }
}


template<typename KeyType>
bool BTree<KeyType>::insert(KeyType key) {
    BTNode *p;
    size_t idx;
    if(_searchBTree(key, p, idx)) return false;
    _insertBTree(p, idx, key);
    return true;
}


/*
 * 从p结点删除key[idx]和它的孩子指针ptr[idx]
 */
#define _removeChildWithIdx(p, idx) do { \
    memmove(&(p->key[idx]), &(p->key[idx + 1]), (p->keynum - idx) * sizeof(KeyType));\
    memmove(&(p->ptr[idx]), &(p->ptr[idx + 1]), (p->keynum - idx) * sizeof(BTNode *));\
    p->keynum--; \
 }while(0)


/*
 * 右子树边最小的关键字。左边是小于关键字的，右边是大于等于关键字的。
 */

template<typename KeyType>
inline void BTree<KeyType>::_substitution(BTree<KeyType>::BTNode *p, size_t idx) {
    BTNode *q;
    for(q = p->ptr[idx]; q->ptr[0] != NULL; q = q->ptr[0]);
    p->key[idx] = q->key[1];                            //复制关键字值
}

/*
 * 
 */

template<typename KeyType>
void BTree<KeyType>::_moveRight(BTree<KeyType>::BTNode *p, size_t idx) {
/*将双亲结点p中的最后一个关键字移入右结点q中
将左结点aq中的最后一个关键字移入双亲结点p中*/ 
    BTNode *q = p->ptr[idx];
    BTNode *aq = p->ptr[idx - 1];

    memmove(&(q->key[2]), &(q->key[1]), q->keynum * sizeof(KeyType)); //将右兄弟q中所有关键字向后移动一位
    memmove(&(q->ptr[1]), q->ptr, (q->keynum + 1) * sizeof(BTNode *));

     //从双亲结点p移动关键字到右兄弟q中
    q->key[1] = p->key[idx];
    q->keynum++;

    p->key[idx] = aq->key[aq->keynum];                  //将左兄弟aq中最后一个关键字移动到双亲结点p中
    q->ptr[0] = aq->ptr[aq->keynum];
    aq->keynum--;
}

/*
 * 将双亲结点p中的第一个关键字移入左结点aq中，将右结点q中的第一个关键字移入双亲结点p中
 */

template<typename KeyType>
void BTree<KeyType>::_moveLeft(BTree<KeyType>::BTNode *p, size_t idx) {

    BTNode *q = p->ptr[idx];
    BTNode *aq = p->ptr[idx - 1];

    aq->keynum++;                                   //把双亲结点p中的关键字移动到左兄弟aq中
    aq->key[aq->keynum] = p->key[idx]; 
    aq->ptr[aq->keynum] = q->ptr[0];

    p->key[idx] = q->key[1];                            //把右兄弟q中的关键字移动到双亲节点p中

    q->keynum--;
    memmove(&(q->key[1]), &(q->key[2]), q->keynum * sizeof(KeyType));       //将右兄弟q中所有关键字向前移动一位
    memmove(q->ptr, &(q->ptr[1]), (q->keynum + 1) * sizeof(BTNode *));
    
}

/*
 * 将双亲结点p、右结点q合并入左结点aq，并调整双亲结点p中的剩余关键字的位置
 */

template<typename KeyType>
void BTree<KeyType>::_combine(BTree<KeyType>::BTNode *p, size_t idx) {
    BTNode *q = p->ptr[idx];                            
    BTNode *aq = p->ptr[idx - 1];

    aq->keynum++;                                  //将双亲结点的关键字p->key[i]插入到左结点aq     
    aq->key[aq->keynum] = p->key[idx];
    aq->ptr[aq->keynum] = q->ptr[0];
    if(aq->ptr[aq->keynum]){
        aq->ptr[aq->keynum]->parent = aq;
    }

    for(size_t j = 1; j <= q->keynum; ++j){                      //将右结点q中的所有关键字插入到左结点aq 
        aq->keynum++;
        aq->key[aq->keynum] = q->key[j];
        aq->ptr[aq->keynum] = q->ptr[j];
        if(q->ptr[j] != NULL)
            aq->ptr[aq->keynum]->parent = aq;
    }

    for(size_t j = idx; j < p->keynum; ++j){                       //将双亲结点p中的p->key[i]后的所有关键字向前移动一位 
        p->key[j] = p->key[j + 1];
        p->ptr[j] = p->ptr[j + 1];
    }
    p->keynum--;                                    //修改双亲结点p的keynum值 
    delete q;                                        //释放空右结点q的空间
}

/* 
 * p节点第idx个key
 */

template<typename KeyType>
void BTree<KeyType>::_adjustBTree(BTNode *p, size_t idx){
    if(idx == 0){                                        //删除的是最左边关键字
        if(p->ptr[1]->keynum > min_keynum)                   //右结点可以借
            _moveLeft(p, 1);
        else                                         //右兄弟不够借 
            _combine(p, 1);
    }else if(idx == p->keynum) {                           //删除的是最右边关键字
        if(p->ptr[idx - 1]->keynum > min_keynum)                 //左结点可以借 
            _moveRight(p, idx);
        else                                        //左结点不够借 
            _combine(p, idx);
    }else if(p->ptr[idx - 1]->keynum > min_keynum)                //删除关键字在中部且左结点够借 
        _moveRight(p, idx);
    else if(p->ptr[idx + 1]->keynum > min_keynum)                //删除关键字在中部且右结点够借 
        _moveLeft(p, idx+1);
    else                                            //删除关键字在中部且左右结点都不够借
        _combine(p, idx);
}


/*
 * 在结点p中查找并删除关键字k
 */

template<typename KeyType>
bool BTree<KeyType>::_btNodeDelete(BTree<KeyType>::BTNode *p, KeyType key) {
                                  //查找标志 
    if(p == NULL)                                     
        return false;
    else{
        size_t idx;
        bool found = _searchNode(p, key, idx);                //返回查找结果 
        if(found){                           //查找成功 
            if(p->ptr[idx] != NULL){             //删除的是非叶子结点
                _substitution(p, idx);                //寻找相邻关键字(右子树中最小的关键字) 
                _btNodeDelete(p->ptr[idx], p->key[idx]);  //执行删除操作（这里可优化）
            }else{                                    //叶子节点
                _removeChildWithIdx(p, idx);                        //从结点p中位置i处删除关键字
            }
        }else
            found = _btNodeDelete(p->ptr[idx], key);    //沿孩子结点递归查找并删除关键字key

        if(p->ptr[idx] != NULL && p->ptr[idx]->keynum < min_keynum)               //删除后关键字个数小于min_keynum
                _adjustBTree(p, idx);                   //调整B树

        return found;

    }
}


template<typename KeyType>
void BTree<KeyType>::del(KeyType key){
//构建删除框架，执行删除操作  
    bool r = _btNodeDelete(root, key);                        //删除关键字k 

    if(r && root->keynum == 0){     //当根只有一个key且儿子发生combine时才会发生这种情况  
        BTNode *p = root;
        root = root->ptr[0];
        
        if(root){
            root->parent = NULL;
        }
        delete p;
    }
}


template<typename KeyType>
void BTree<KeyType>::_destroyBTree(BTNode* &p){
    if(p == NULL) return;
    //递归释放B树                                   //B树不为空  
    for(size_t i = 0; i <= p->keynum; ++i){                  //递归释放每一个结点 
        _destroyBTree(p->ptr[i]);  
    }  
    delete p;  
    p = NULL;  
}  


template<typename KeyType>
void BTree<KeyType>::traverse() {
    if(root == NULL){
        printf("  B tree is empty\r\n");
        return;
    }

    std::queue<std::pair<BTNode*, BTNode*> > que;
    que.push({root,NULL});
    que.push({NULL,NULL});

    while(!que.empty()) {
        std::pair<BTNode*, BTNode*> q = que.front(); que.pop();
        if(q.first == NULL) {
            printf("\r\n");
            if(!que.empty())
                que.push({NULL,NULL});
            continue;
        }
        BTNode *p = q.first;

        printf(" %d [", p->keynum);
        for(size_t i = 1; i <= p->keynum; ++i){
            //TODO:这个应该加个接口
            printf(" %d ", p->key[i]);
        }
        printf("]");


        if(p->ptr[0] == NULL) continue;
        for(size_t i = 0; i <= p->keynum; ++i){ 
            que.push({p->ptr[i], p});
        }
    }
 }
} //namespace btree

#ifdef DEBUG
void test1(){
    btree::BTree<int> tree(50);
    //BTree<int> tree(50);
    while(true){
        int MOD=1000000;
        printf("===============================\r\n");
        int n = rand() % 100;
        printf("n=%d\r\n", n);
        for(int i = 0; i < n; ++i){
            int a = rand() % MOD;
            printf(" %d ",a);
            tree.insert(a);
        }   
        printf("\r\n");
        
        tree.traverse();

        int m = rand() % 100;
        
        printf("m = %d\r\n", m);
        for(int i = 0; i < m; ++i){
            int a = rand() % MOD;
            printf(" %d ",a);
            tree.del(a);
        }
        printf("\r\n");

        tree.traverse();
    }

}

void test2(){
    int i, key, n; 
    system("color 70");
    btree::BTree<int> tree(5);
    //BTree<int> tree(5);
    while(1){
    printf("current tree is: \r\n");
    tree.traverse(); 
    printf("\r\n");
    printf("=============Operation Table=============\r\n");
    printf("   1.Create   2.Insert    3.Delete    \r\n");
    printf("   4.Destroy  5.Exit      \r\n");
    printf("=========================================\r\n");
    printf("Enter number to choose operation:_____\b\b\b");
    scanf("%d",&i);
    switch(i){
        case 1:{
            int m;
            printf("Enter the parameter m\r\n");
            scanf("%d", &m);
            tree = btree::BTree<int>(m);
            printf("InitBTree successfully.\r\n");
            break;
        }
        case 2:{
            printf("Enter how many keys to insert and keys:_____\b\b\b");
            scanf("%d",&n);
            
            for(int i = 0;i < n; ++i){
                scanf("%d",&key);
                bool ex = tree.insert(key);
            }

            printf("Insert successfully\r\n");
            break;
        }
        case 3:{
            printf("Enter how mant keys to delete and keys:_____\b\b\b");
            scanf("%d",&n);
            for(int i = 0;i < n; ++i){
                scanf("%d",&key);
                tree.del(key);
            }
            printf("DeleteBTree successfully.\r\n");
            break;
        }
        case 4:{
            tree = btree::BTree<int>(5);
            //tree = BTree<int>(5);
            printf("DestroyBTree successfully.\r\n");
            break;
        }
        case 5:{
            exit(-1);               
            break;
        }
    }
 }
}

int main(){
    test1();
    return 0;
}
#endif