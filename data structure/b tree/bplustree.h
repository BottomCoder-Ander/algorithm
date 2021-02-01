#ifndef _BPLUS_TREE_H
#define _BPLUS_TREE_H

#include <unistd.h>

/* 5 node caches are needed at least for self, left and right sibling, sibling
 * of sibling, parent and node seeking */
#define MIN_CACHE_NUM 5

#define list_entry(ptr, type, member) \
        ((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

#define list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
                pos = n, n = pos->next)

typedef int bptree_key_t;
typedef long bptree_val_t;

struct list_head {
        struct list_head *prev, *next;
};

static inline void list_init(struct list_head *link)
{
        link->prev = link;
        link->next = link;
}

/* add a node link between prev and next. prev -> link -> next */
static inline void
__list_add(struct list_head *link, struct list_head *prev, struct list_head *next)
{
        link->next = next;
        link->prev = prev;
        next->prev = link;
        prev->next = link;
}

/* delete node between prev and next */
/* TODO: explain why it does not free the node */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
        prev->next = next;
        next->prev = prev;
}

/* add a node link after prev */
static inline void list_add(struct list_head *link, struct list_head *prev)
{
        __list_add(link, prev, prev->next);
}

/* add a node link before head  (add a node in the tail) */
static inline void list_add_tail(struct list_head *link, struct list_head *head)
{
	__list_add(link, head->prev, head);
}

/* delete a node link, but do not free */
static inline void list_del(struct list_head *link)
{
        __list_del(link->prev, link->next);
        list_init(link);
}

/* test if a list is empty */
static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

typedef struct bplus_node {
        /* offset in the file of this node */
        off_t self;
        /* parent offset */
        off_t parent;
        /* prev node (left sibling) offset */
        off_t prev;
        /* next node (right sibling) offset */
        off_t next;
        /* If it's a leaf, type = 0, otherwise type = 1 */
        int type;
        /* As leaf node, children specifies count of entries,
         * As non-leaf node, it specifies count of children(branches) */
        int children;
} bplus_node;

/*
struct bplus_non_leaf {
        off_t self;
        off_t parent;
        off_t prev;
        off_t next;
        int type;
        int children;
        bptree_key_t key[BPLUS_MAX_ORDER - 1];
        off_t sub_ptr[BPLUS_MAX_ORDER];
};
struct bplus_leaf {
        off_t self;
        off_t parent;
        off_t prev;
        off_t next;
        int type;
        int entries;
        bptree_key_t key[BPLUS_MAX_ENTRIES];
        long data[BPLUS_MAX_ENTRIES];
};
*/

typedef struct free_block {
        struct list_head link;
        off_t offset;
} free_block;

struct bplus_tree {
        char *caches;
        /* a flag marks if cache[i] is used */
        int used[MIN_CACHE_NUM];
        /* filename of this bplus tree */
        char filename[1024]; 
        /* current file discriptor */
        int fd;
        /* layer number? */
        int level;
        /* root offset in the file */
        off_t root;
        off_t file_size;
        /* a list contain free blocks in the file (this block can be use again) */
        struct list_head free_blocks;
};

void bplus_tree_dump(struct bplus_tree *tree);
long bplus_tree_get(struct bplus_tree *tree, bptree_key_t key);
int bplus_tree_put(struct bplus_tree *tree, bptree_key_t key, long data);
long bplus_tree_get_range(struct bplus_tree *tree, bptree_key_t key1, bptree_key_t key2);
struct bplus_tree *bplus_tree_init(char *filename, int block_size);
void bplus_tree_deinit(struct bplus_tree *tree);
int bplus_open(char *filename);
void bplus_close(int fd);

#endif  /* _BPLUS_TREE_H */
