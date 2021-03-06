#include "gtk-ml.h"

#define GTKML_H_BITS 5
#define GTKML_H_SIZE (1 << GTKML_H_BITS)
#define GTKML_H_MASK (GTKML_H_SIZE - 1)

typedef enum GtkMl_HashSetNodeKind {
    GTKML_HS_LEAF,
    GTKML_HS_BRANCH,
} GtkMl_HashSetNodeKind;

typedef struct GtkMl_HLeaf {
    GtkMl_S *key;
} GtkMl_HLeaf;

typedef struct GtkMl_HBranch {
    GtkMl_HashSetNode **nodes;
} GtkMl_HBranch;

typedef union GtkMl_HUnion {
    GtkMl_HLeaf h_leaf;
    GtkMl_HBranch h_branch;
} GtkMl_HUnion;

struct GtkMl_HashSetNode {
    int rc;
    GtkMl_HashSetNodeKind kind;
    GtkMl_HUnion value;
};

GTKML_PRIVATE GtkMl_HashSetNode *new_leaf(GtkMl_S *key);
GTKML_PRIVATE GtkMl_HashSetNode *new_branch();
GTKML_PRIVATE GtkMl_HashSetNode *copy_node(GtkMl_HashSetNode *node);
GTKML_PRIVATE void del_node(GtkMl_HashSetNode *node);
GTKML_PRIVATE GtkMl_S *insert(GtkMl_HashSetNode **out, size_t *inc, GtkMl_HashSetNode *node, GtkMl_S *key, GtkMl_Hash hash, uint32_t shift);
GTKML_PRIVATE GtkMl_S *get(GtkMl_HashSetNode *node, GtkMl_S *key, GtkMl_Hash hash, uint32_t shift);
GTKML_PRIVATE GtkMl_S *delete(GtkMl_HashSetNode **out, size_t *dec, GtkMl_HashSetNode *node, GtkMl_S *key, GtkMl_Hash hash, uint32_t shift);
GTKML_PRIVATE GtkMl_VisitResult foreach(GtkMl_HashSet *hs, GtkMl_HashSetNode *node, GtkMl_HashSetFn fn, void *data);

void gtk_ml_new_hash_set(GtkMl_HashSet *hs) {
    hs->root = NULL;
    hs->len = 0;
}

void gtk_ml_del_hash_set(GtkMl_HashSet *hs) {
    del_node(hs->root);
}

size_t gtk_ml_hash_set_len(GtkMl_HashSet *hs) {
    return hs->len;
}

GTKML_PRIVATE GtkMl_VisitResult fn_concat(GtkMl_HashSet *ht, GtkMl_S *value, void *data) {
    (void) ht;

    GtkMl_HashSet *dest = data;

    GtkMl_HashSet new;
    gtk_ml_hash_set_insert(&new, dest, value);
    *dest = new;
    
    return GTKML_VISIT_RECURSE;
}

void gtk_ml_hash_set_concat(GtkMl_HashSet *out, GtkMl_HashSet *lhs, GtkMl_HashSet *rhs) {
    out->root = copy_node(lhs->root);
    out->len = lhs->len;

    gtk_ml_hash_set_foreach(rhs, fn_concat, out);
}

GtkMl_S *gtk_ml_hash_set_insert(GtkMl_HashSet *out, GtkMl_HashSet *hs, GtkMl_S *key) {
    out->root = NULL;
    out->len = hs->len;

    GtkMl_Hash hash;
    if (!gtk_ml_hash(&hash, key)) {
        return NULL;
    }
    return insert(&out->root, &out->len, hs->root, key, hash, 0);
}

GtkMl_S *gtk_ml_hash_set_get(GtkMl_HashSet *hs, GtkMl_S *key) {
    GtkMl_Hash hash;
    if (!gtk_ml_hash(&hash, key)) {
        return NULL;
    }
    return get(hs->root, key, hash, 0);
}

gboolean gtk_ml_hash_set_contains(GtkMl_HashSet *hs, GtkMl_S *key) {
    GtkMl_Hash hash;
    if (!gtk_ml_hash(&hash, key)) {
        return 0;
    }
    return get(hs->root, key, hash, 0) != NULL;
}

GtkMl_S *gtk_ml_hash_set_delete(GtkMl_HashSet *out, GtkMl_HashSet *hs, GtkMl_S *key) {
    out->root = NULL;
    out->len = hs->len;

    GtkMl_Hash hash;
    if (!gtk_ml_hash(&hash, key)) {
        return NULL;
    }
    return delete(&out->root, &out->len, hs->root, key, hash, 0);
}

void gtk_ml_hash_set_foreach(GtkMl_HashSet *hs, GtkMl_HashSetFn fn, void *data) {
    foreach(hs, hs->root, fn, data);
}

GtkMl_HashSetNode *new_leaf(GtkMl_S *key) {
    GtkMl_HashSetNode *node = malloc(sizeof(GtkMl_HashSetNode));
    node->rc = 1;
    node->kind = GTKML_HS_LEAF;
    node->value.h_leaf.key = key;
    return node;
}

GtkMl_HashSetNode *new_branch() {
    GtkMl_HashSetNode *node = malloc(sizeof(GtkMl_HashSetNode));
    node->rc = 1;
    node->kind = GTKML_HS_BRANCH;
    node->value.h_branch.nodes = malloc(sizeof(GtkMl_HashSetNode *) * GTKML_H_SIZE);
    memset(node->value.h_branch.nodes, 0, sizeof(GtkMl_HashSetNode *) * GTKML_H_SIZE);
    return node;
}

GtkMl_HashSetNode *copy_node(GtkMl_HashSetNode *node) {
    if (!node) {
        return NULL;
    }

    ++node->rc;

    return node;
}

void del_node(GtkMl_HashSetNode *node) {
    if (!node) {
        return;
    }

    --node->rc;
    if (!node->rc) {
        switch (node->kind) {
        case GTKML_HS_LEAF:
            break;
        case GTKML_HS_BRANCH:
            for (size_t i = 0; i < GTKML_H_SIZE; i++) {
                del_node(node->value.h_branch.nodes[i]);
            }
            free(node->value.h_branch.nodes);
            break;
        }
        free(node);
    }
}

GtkMl_S *insert(GtkMl_HashSetNode **out, size_t *inc, GtkMl_HashSetNode *node, GtkMl_S *key, GtkMl_Hash hash, uint32_t shift) {
    if (!node) {
        ++*inc;
        *out = new_leaf(key);
        return NULL;
    }

    switch (node->kind) {
    case GTKML_HS_LEAF:
        if (gtk_ml_equal(key, node->value.h_leaf.key)) {
            *out = new_leaf(key);
            return node->value.h_leaf.key;
        } else {
            *out = new_branch();

            GtkMl_Hash _hash;
            gtk_ml_hash(&_hash, node->value.h_leaf.key);
            uint32_t _idx = (_hash >> shift) & GTKML_H_MASK;
            (*out)->value.h_branch.nodes[_idx] = copy_node(node);

            uint32_t idx = (hash >> shift) & GTKML_H_MASK;
            return insert(&(*out)->value.h_branch.nodes[idx], inc, (*out)->value.h_branch.nodes[idx], key, hash, shift + GTKML_H_BITS);
        }
    case GTKML_HS_BRANCH: {
        *out = new_branch();
        for (size_t i = 0; i < GTKML_H_SIZE; i++) {
            (*out)->value.h_branch.nodes[i] = copy_node(node->value.h_branch.nodes[i]);
        }
        uint32_t idx = (hash >> shift) & GTKML_H_MASK;
        return insert(&(*out)->value.h_branch.nodes[idx], inc, node->value.h_branch.nodes[idx], key, hash, shift + GTKML_H_BITS);
    }
    }
}

GtkMl_S *get(GtkMl_HashSetNode *node, GtkMl_S *key, GtkMl_Hash hash, uint32_t shift) {
    if (!node) {
        return NULL;
    }

    switch (node->kind) {
    case GTKML_HS_LEAF:
        if (gtk_ml_equal(node->value.h_leaf.key, key)) {
            return node->value.h_leaf.key;
        } else {
            return NULL;
        }
    case GTKML_HS_BRANCH: {
        uint32_t idx = (hash >> shift) & GTKML_H_MASK;
        return get(node->value.h_branch.nodes[idx], key, hash, shift + GTKML_H_BITS);
    }
    }
}

GtkMl_S *delete(GtkMl_HashSetNode **out, size_t *dec, GtkMl_HashSetNode *node, GtkMl_S *key, GtkMl_Hash hash, uint32_t shift) {
    if (!node) {
        return NULL;
    }

    switch (node->kind) {
    case GTKML_HS_LEAF:
        if (gtk_ml_equal(node->value.h_leaf.key, key)) {
            --*dec;
            del_node(*out);
            *out = NULL;
            return node->value.h_leaf.key;
        } else {
            return NULL;
        }
    case GTKML_HS_BRANCH: {
        *out = new_branch();
        for (size_t i = 0; i < GTKML_H_SIZE; i++) {
            (*out)->value.h_branch.nodes[i] = copy_node(node->value.h_branch.nodes[i]);
        }
        uint32_t idx = (hash >> shift) & GTKML_H_MASK;
        return delete(&(*out)->value.h_branch.nodes[idx], dec, node->value.h_branch.nodes[idx], key, hash, shift + GTKML_H_BITS);
    }
    }
}

GtkMl_VisitResult foreach(GtkMl_HashSet *hs, GtkMl_HashSetNode *node, GtkMl_HashSetFn fn, void *data) {
    if (!node) {
        return GTKML_VISIT_RECURSE;
    }

    switch (node->kind) {
    case GTKML_HS_LEAF:
        return fn(hs, node->value.h_leaf.key, data);
    case GTKML_HS_BRANCH: {
        for (size_t i = 0; i < GTKML_H_SIZE; i++) {
            switch (foreach(hs, node->value.h_branch.nodes[i], fn, data)) {
            case GTKML_VISIT_RECURSE:
                continue;
            case GTKML_VISIT_CONTINUE:
                return GTKML_VISIT_RECURSE;
            case GTKML_VISIT_BREAK:
                return GTKML_VISIT_BREAK;
            }
        }
        return GTKML_VISIT_RECURSE;
    }
    }
}
