#include "disjoint_set.h"
#include <stdio.h>
#include <stdlib.h>

DSNode *make_node(long value) {
    DSNode *node = (DSNode *)malloc(sizeof(DSNode));
    if (!node) {
        printf("Failed to allocate memory for node\n");
        return NULL;
    }

    node->value = value;
    node->parent = node;
    node->rank = 0;

    return node;
}

DisjointSet *dsu_create(){
    DisjointSet *dsu = (DisjointSet *)malloc(sizeof(DisjointSet));
    if (!dsu) {
        printf("Failed to allocate memory for dsu\n");
        return NULL;
    }

    dsu->trees = createHashTable(sizeof(long), sizeof(DSNode*));
    if (!dsu->trees) {
        printf("Failed to allocate memory for trees\n");
        free(dsu);
        return NULL;
    }

    return dsu;
}

DisjointSet *dsu_create_from_hash_table(HashTable *nodes){
    if (!nodes) {
        printf("Invalid data for creating dsu\n");
        return NULL;
    }

    DisjointSet *dsu = (DisjointSet *)malloc(sizeof(DisjointSet));
    if (!dsu) {
        printf("Failed to allocate memory for dsu\n");
        return NULL;
    }

    dsu->trees = nodes;

    return dsu;
}

DSNode *find_set(DSNode *node){
    if (!node) {
        printf("Invalid data for finding set\n");
        return NULL;
    }

    if (node->parent != node) {
        node->parent = find_set(node->parent);
    }

    return node->parent;
}

DSNode *find_set_from_key(DisjointSet *dsu, long key){
    if (!dsu) {
        printf("Invalid data for finding set\n");
        return NULL;
    }

    DSNode **node_ptr = (DSNode **)getItemHashTable(dsu->trees, &key, hashLong, longEquals);
    if (!node_ptr || !*node_ptr) {
        printf("Failed to get node\n");
        return NULL;
    }

    return find_set(*node_ptr);
}

void union_sets(DSNode *a, DSNode *b){
    if (!a || !b) {
        printf("Invalid data for union sets\n");
        return;
    }

    DSNode *root_a = find_set(a);
    DSNode *root_b = find_set(b);

    if (root_a == root_b) {
        return;
    }

    if (root_a->rank < root_b->rank) {
        root_a->parent = root_b;
    }
    else if (root_a->rank > root_b->rank) {
        root_b->parent = root_a;
    }
    else {
        root_b->parent = root_a;
        root_a->rank++;
    }
}

void freeDisjointSet(DisjointSet *dsu) {
    if (dsu) {
        if (dsu->trees) {
            size_t item_size = 1 + dsu->trees->key_size + dsu->trees->val_size;
            char *items = dsu->trees->values->data;

            for (size_t i = 0; i < dsu->trees->capacity; i++) {
                unsigned char *state = (unsigned char*)(items + i * item_size);
                if (*state == SLOT_OCCUPIED) {
                    DSNode **node_ptr = (DSNode **)(items + i * item_size + 1 + dsu->trees->key_size);
                    if (node_ptr && *node_ptr) {
                        free(*node_ptr);
                    }
                }
            }

            freeHashTable(dsu->trees);
        }

        free(dsu);
    }
}
