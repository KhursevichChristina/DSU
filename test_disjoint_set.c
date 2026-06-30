#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disjoint_set.h"

// Вспомогательная функция для логирования
void log_result(const char* task, const char* message) {
    printf("[%s] %s\n", task, message);
}


int main() {
    printf("Starting testing\n");
    
    log_result("make_node", "Starting tests");

    DSNode *node = make_node(5);
    assert(node != NULL);
    assert(node->value == 5);
    assert(node->parent == node);
    free(node);

    node = make_node(0);
    assert(node != NULL);
    assert(node->value == 0);
    assert(node->parent == node);
    free(node);
    
    log_result("make_node", "Tests passed");

    log_result("dsu_create", "Starting tests");

    DisjointSet *dsu = dsu_create();
    assert(dsu != NULL);
    assert(dsu->trees != NULL);
    assert(dsu->trees->size == 0);
    assert(dsu->trees->key_size == sizeof(long));
    assert(dsu->trees->val_size == sizeof(DSNode*));
    freeDisjointSet(dsu);
    
    log_result("dsu_create", "Tests passed");

    log_result("dsu_create_from_hash_table", "Starting tests");

    HashTable *table = createHashTable(sizeof(long), sizeof(DSNode*));
    long key1 = 10;
    long key2 = 20;
    DSNode *node1 = make_node(key1);
    DSNode *node2 = make_node(key2);
    setItemHashTable(table, &key1, &node1, hashLong, longEquals);
    setItemHashTable(table, &key2, &node2, hashLong, longEquals);

    dsu = dsu_create_from_hash_table(table);
    assert(dsu != NULL);
    assert(dsu->trees != NULL);
    assert(dsu->trees == table);
    assert(find_set_from_key(dsu, key1) == node1);
    assert(find_set_from_key(dsu, key2) == node2);

    assert(find_set_from_key(NULL, key1) == NULL);
    assert(dsu_create_from_hash_table(NULL) == NULL);
    assert(find_set_from_key(dsu, 200) == NULL);

    freeDisjointSet(dsu);

    log_result("dsu_create_from_hash_table", "Tests passed");

    log_result("find_set", "Starting tests");

    node = make_node(5);
    assert(find_set(node) == node);
    assert(find_set(NULL) == NULL);
    free(node);

    log_result("find_set", "Tests passed");

    log_result("union_sets", "Starting tests");

    node1 = make_node(key1);
    node2 = make_node(key2);
    DSNode *node3 = make_node(30);

    assert(find_set(node1) != find_set(node2));

    union_sets(node1, node2);
    union_sets(node2, node3);
    assert(find_set(node1) == find_set(node2));
    assert(find_set(node1) == find_set(node3));
    assert(find_set(node3) == find_set(node2));

    union_sets(NULL, node2);
    union_sets(node1, NULL);
    union_sets(NULL, NULL);

    DSNode *root1 = find_set(node1);
    union_sets(node1, node2);
    DSNode *root2 = find_set(node1);
    assert(root1 == root2);
    assert(find_set(node1) == find_set(node2));

    free(node1);
    free(node2);
    free(node3);

    log_result("union_sets", "Tests passed");

    printf("All tests passed\n");
}
