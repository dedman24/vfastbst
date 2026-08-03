#ifndef VFASTBST_H_INCLUDED
#define VFASTBST_H_INCLUDED

// VFASTBST v1.0
// vfastbst implements a fast, lockless, insert-only binary search tree.
//
// FEATURES:
// fast lockless addition of an entry to the tree; addition also tells you whether or not the entry already existed within the tree.
// the only way to tell if an entry exists within a tree or not is to add it; read operations are useless because by the time you've read the entry could've already been added.
//
// lockless BSTs can be implemented quite easily, because a bst only holds 1 entry. were they to hold 2, there'd be cases where insertion doesn't allocate.
//
// PUT:
//  traverse down tree by doing CAS(&node->leaf, &next /* initialised to NULL */, new).
//  if it fails, go down a node.
//  if it succeeds, return true.
// clearly, the node has to be preallocated ughhh...
// we can preallocate the node as late as possible. actual algorithm is a bit more complex; see for yourself.

// stdlib includes.
#include "stddef.h"         // for NULL, size_t.
#include "stdint.h"         // for uintptr_t.
#include "stdlib.h"         // for calloc, free.
#include "string.h"         // for strcmp.
#include "stdbool.h"        // for bool type.
#include "stdatomic.h"      // for atomic ops.

typedef struct{ struct vfastbst_nodeS* _Atomic node; } vfastbstT;   // all types terminated with T and not _t to be annoyingly POSIX-compliant. fuck you POSIX why'd you do this???
bool vfastbst_put(vfastbstT* const restrict bst, const char* const restrict key);

# ifdef VFASTBST_IMPLEMENTATION

typedef struct vfastbst_nodeS{
  char* key;
  struct vfastbst_nodeS* _Atomic leaf[2];
} vfastbst_nodeT;

static vfastbst_nodeT* vfastbst_node_init(char* const restrict key){
  vfastbst_nodeT* const restrict node = calloc(1, sizeof(*node));
  node->key = key;

  return node;
}

static void vfastbst_node_destroy(vfastbst_nodeT* const restrict node){
  if(node) free(node);
}

bool vfastsearch_put(vfastbstT* const restrict bst, char* const restrict key){
  vfastbst_nodeT* t_new = NULL;
// inserts if tree is empty.
  vfastbst_nodeT* restrict node = atomic_load(&bst->node);
  if(!node){
    t_new = vfastbst_node_init(key);

    vfastbst_nodeT* t_null = NULL;
    if(atomic_compare_exchange_strong(&bst->node, &t_null, t_new)) return true;
    else node = t_null;
  }

  while(1){
    const int r = strcmp(node->key, key);
    if(!r){
      vfastbst_node_destroy(t_new);
      return false;
    }
    const size_t ch_pick = r>0;

    vfastbst_nodeT* const restrict pick = atomic_load(&node->leaf[ch_pick]);
    if(pick){ node = pick; continue; }
    if(!t_new) t_new = vfastbst_node_init(key);

    vfastbst_nodeT* t_null = NULL;
    if(atomic_compare_exchange_strong(&node->leaf[ch_pick], &t_null, t_new)) return true;
    else node = t_null;
  }
}

# endif
#endif
