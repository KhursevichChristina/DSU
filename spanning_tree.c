// Функция для построения минимального остовного дерева из ориентированного графа
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lab4/hash_table/generic.h"
#include "KhursevichChristina_lab3/graph.h"
#include "disjoint_set.h"
// #include "doubly_list/generic.h"

#define MAX_LINE 256

typedef struct NodeWithDistance {
    long node_id;
    double distance;
} NodeWithDistance;

typedef struct EdgeForKruskal {
    long long from;
    long long to;
    double length;
} EdgeForKruskal;

// Жулик своровал весь код! Помогите Даше его восстановить

double calculate_distance(double lat1, double lon1, double lat2, double lon2) {
    double x = (lat1 - lat2) * 111320.0;
    double y = (lon1 - lon2) * 111320.0 * cos((lat1 + lat2) / 2.0 * 3.14159265358979323846 / 180.0);
    return sqrt(x * x + y * y);
}

int cmpDistance(const void *a, const void *b) {
    NodeWithDistance *node1 = (NodeWithDistance*)a;
    NodeWithDistance *node2 = (NodeWithDistance*)b;
    if (node1->distance < node2->distance) {
        return -1;
    }
    if (node1->distance > node2->distance) {
        return 1;
    }
    return 0;
}

int cmpEdges(const void *a, const void *b) {
    EdgeForKruskal *edge1 = (EdgeForKruskal*)a;
    EdgeForKruskal *edge2 = (EdgeForKruskal*)b;
    if (edge1->length < edge2->length) {
        return -1;
    }
    if (edge1->length > edge2->length) {
        return 1;
    }
    return 0;
}

Graph *read_buildings(char *path_nodes) {
    FILE *file = fopen(path_nodes, "r");
    if (!file) {
        printf("Failed to open file\n");
        return NULL;
    }

    Graph *graph = createGraph();
    if (!graph) {
        fclose(file);
        return NULL;
    }

    char line[MAX_LINE];
    fgets(line, sizeof(line), file);
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        long long id;
        double lat;
        double lon;

        if (sscanf(line, "%lld\t%*[^\t]\t%lf\t%lf", &id, &lat, &lon) == 3) {
            GraphNode *node = createNode(lat, lon, id);
            if (node) {
                addNode(graph, node);
                count++;
            }
        }
    }

    fclose(file);
    printf("Graph with %d nodes\n", count);
    return graph;
}

void build_edges(Graph *graph, unsigned char max_edges) {
    if (!graph || !graph->nodes || graph->node_count == 0) {
        printf("Invalid data for building edges\n");
        return;
    }

    size_t n = graph->node_count;
    NodeWithDistance *distances = malloc((n - 1) * sizeof(NodeWithDistance));
    if (!distances) {
        printf("Failed to allocate memory for distances\n");
        return;
    }

    for (size_t i = 0; i < n; i++) {
        GraphNode **node_ptr1 = (GraphNode**)getVectorItem(graph->nodes, i);
        if (!node_ptr1 || !*node_ptr1) {
            continue;
        }
        GraphNode *node1 = *node_ptr1;

        int count = 0;
        for (size_t j = 0; j < n; j++) {
            if (i == j) {
                continue;
            }

            GraphNode **node_ptr2 = (GraphNode**)getVectorItem(graph->nodes, j);
            if (!node_ptr2 || !*node_ptr2) {
                continue;
            }
            GraphNode *node2 = *node_ptr2;

            distances[count].node_id = node2->node_id;
            distances[count].distance = calculate_distance(node1->lat, node1->lon, node2->lat, node2->lon);
            count++;
        }

        qsort(distances, count, sizeof(NodeWithDistance), cmpDistance);

        int k = (max_edges < count) ? max_edges : count;
        for (int j = 0; j < k; j++) {
            addEdge(graph, node1->node_id, distances[j].node_id, distances[j].distance, 0, NULL);
        }
    }

    free(distances);
    printf("%d edges\n", graph->edges_count);
}

Vector *kruskal_mst(Graph *graph) {
    if (!graph || !graph->nodes || graph->node_count == 0) {
        printf("Empty graph\n");
        return NULL;
    }

    printf("Start Kruskal\n");
    printf("All edges: %d\n", graph->edges_count);

    Vector *all_edges = createVector(sizeof(EdgeForKruskal));
    if (!all_edges) {
        return NULL;
    }

    for (int i = 0; i < graph->node_count; i++) {
        GraphNode **ptr = (GraphNode**)getVectorItem(graph->nodes, i);
        if (!ptr || !*ptr) {
            continue;
        }

        GraphNode *node1 = *ptr;

        for (size_t j = 0; j < node1->edges->size; j++) {
            Edge *edge = (Edge*)getVectorItem(node1->edges, j);
            if (!edge) {
                continue;
            }

            if (node1->node_id < edge->id_to) {
                EdgeForKruskal edgeKruskal;
                edgeKruskal.from = node1->node_id;
                edgeKruskal.to = edge->id_to;
                edgeKruskal.length = edge->length;

                appendVectorItem(all_edges, &edgeKruskal);
            }
        }
    }

    qsort(all_edges->data, all_edges->size, sizeof(EdgeForKruskal), cmpEdges);
    printf("Edges are sorted\n");

    DisjointSet *dsu = dsu_create();
    if (!dsu) {
        vectorFree(all_edges);
        return NULL;
    }

    for (int i = 0; i < graph->node_count; i++) {
        GraphNode **ptr = (GraphNode**)getVectorItem(graph->nodes, i);
        if (!ptr || !*ptr) {
            continue;
        }

        GraphNode *node = *ptr;
        DSNode *ds_node = make_node(node->node_id);
        if (!ds_node) {
            freeDisjointSet(dsu);
            vectorFree(all_edges);
            return NULL;
        }

        setItemHashTable(dsu->trees, &node->node_id, &ds_node, hashLong, longEquals);
    }
    Vector *mst = createVector(sizeof(EdgeForKruskal));
    if (!mst) {
        printf("Failed to create mst\n");
        freeDisjointSet(dsu);
        vectorFree(all_edges);
        return NULL;
    }

    int edges_added = 0;
    double mst_length = 0.0;

    for (size_t i = 0; i < all_edges->size && edges_added < graph->node_count - 1; i++) {
        EdgeForKruskal *edge = (EdgeForKruskal *)getVectorItem(all_edges, i);
        if (!edge) {
            continue;
        }

        DSNode *from = find_set_from_key(dsu, edge->from);
        DSNode *to = find_set_from_key(dsu, edge->to);

        if (from != to) {
            union_sets(from, to);
            appendVectorItem(mst, edge);
            edges_added++;
            mst_length += edge->length;
        }
    }

    printf("Added edges to MST: %d\n", edges_added);
    printf("MST length: %.2f m\n", mst_length);
    printf("MST built\n");

    freeDisjointSet(dsu);
    vectorFree(all_edges);

    return mst;
}

void write_edges_to_file(Vector *edges, char *path_out) {
    if (!edges || !path_out) {
        printf("Invalid data for writing edges to file\n");
        return;
    }

    FILE *file = fopen(path_out, "w");
    if (!file) {
        printf("Failed to open file\n");
        return;
    }

    fprintf(file, "node_id_from\tnode_id_to\tlength\n");

    for (size_t i = 0; i < edges->size; i++) {
        EdgeForKruskal *edge = (EdgeForKruskal *)getVectorItem(edges, i);
        if (edge) {
            fprintf(file, "%lld\t%lld\t%.6f\n", edge->from, edge->to, edge->length);
        }
    }

    fclose(file);
    printf("MST saved\n");
}

int main() {
    char path_input[] = "data/buildings.csv";
    char path_output[] = "data/spanning_tree.csv";

    printf("Reading buildings from %s\n", path_input);

    Graph* graph = read_buildings(path_input);
    if (!graph) {
        printf("Failed to read buildings\n");
        return -1;
    }

    printf("Building edges\n");
    build_edges(graph, 6);

    printf("Running Kruskal\n");
    Vector* edges = kruskal_mst(graph);

    if (!edges) {
        printf("Failed to build MST\n");
        freeGraph(graph);
        return -1;
    }

    write_edges_to_file(edges, path_output);

    vectorFree(edges);
    freeGraph(graph);

    return 0;
}