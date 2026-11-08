/*
 *  Utility library for personal use.
 *  
 *  Author  : Kinan Principe <kprincipe>
 *  Github  : github.com/kprincipe
 *  Created : 03/10/2026
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Macros for manipulating dynamic arrays
#define DA_INIT_CAP 256

#define da_append(x, xs) \
do { \
    if ((x)->count == 0) { \
        (x)->capacity = DA_INIT_CAP; \
        (x)->items = malloc((DA_INIT_CAP * sizeof(*(x)->items))); \
    } else if ((x)->count == (x)->capacity) { \
        (x)->capacity *= 2; \
        (x)->items = realloc((x)->items, (x)->capacity * sizeof(*(x)->items)); \
    } \
    (x)->items[(x)->count++] = (xs); \
} while(0) \

#define da_pop(x) \
do { \
    if ((x)->count == 0) { \
        if ((x)->capacity == 0) { \
            fprintf(stderr, "%s:%d: error: da_pop() use after free\n", __FILE__, __LINE__); \
            exit(1); \
        } \
        fprintf(stderr, "%s:%d: error: da_pop() popping empty array\n", __FILE__, __LINE__); \
        exit(1); \
    } \
    (x)->count--; \
} while (0) \

#define da_free(x) \
do { \
    free((x)->items); \
    (x)->count = 0; \
    (x)->capacity = 0; \
} while(0) \

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String_Builder;

typedef struct {
    size_t count;
    char *items;
} String_View;

bool util_read_file(String_Builder *sb, const char *filename);

void util_sb_from_cstr(String_Builder *sb, char *str);

String_View util_svn_from_sb(String_Builder *sb, size_t n);
String_View util_svn(char *s, size_t n);

#ifdef UTILS_IMPLEMENTATION

void util_sb_from_cstr(String_Builder *sb, char *str) {
    int c = 0;
    while (str[c] != '\0') {
        da_append(sb, str[c++]);
    }
}

bool util_read_file(String_Builder *sb, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        return false;
    }
    
    char c;
    while((c = fgetc(f)) != EOF) {
        da_append(sb, c);
    }

    fclose(f);

    return true;
}

String_View util_svn_from_sb(String_Builder *sb, size_t n) {
    size_t sz = n;
    if (n > sb->count) {
        sz = sb->count;
    }

    return (String_View) { .items = sb->items, .count = sz };
}

String_View util_svn(char *s, size_t n) {
    size_t sz = n;
    size_t s_size = strlen(s);
    if (n > s_size) {
        sz = s_size;
    }

    return (String_View) { .items = s, .count = sz };
}

#endif // UTILS_IMPLEMENTATION
