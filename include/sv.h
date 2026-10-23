#ifndef SV_H
#define SV_H

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String_Builder;

typedef struct {
    char *items;
    size_t count;
} String_View;

String_View sv_from_parts(char *str, size_t count);
String_View sv_left(String_View sv, size_t n);
String_View sv_right(String_View sv, size_t n);

char *sv_to_cstr(String_View sv);

void sb_append(String_Builder *sb, char *str);

#ifdef SV_IMPLEMENTATION

void sb_append(String_Builder *sb, char *str) {
    size_t expected_capacity = strlen(str);
    size_t current_capacity = (sb->capacity - sb->count);

    if (sb->capacity == 0) {
        sb->items = malloc(4);
        sb->capacity = 4;
    }

    while (current_capacity < expected_capacity) {
        sb->items = realloc(sb->items, sb->capacity + 4);
        sb->capacity += 4;
        current_capacity = (sb->capacity - sb->count);
    }

    memmove(sb->items + sb->count, str, strlen(str));
    sb->count += strlen(str);
}

void sb_free(String_Builder *sb) {
    free(sb->items);
}

String_View sv_from_parts(char *str, size_t count) {
    String_View sv;
    sv.count = count;
    sv.items = str;
    
    return sv;
}

String_View sv_left(String_View sv, size_t n) {
    if (sv.count == 0) {
        return sv;
    }

    if (sv.count < n) {
        sv.items = sv.items + sv.count;
        sv.count = 0;
        return sv;
    }

    sv.items = sv.items + n;
    sv.count -= n;

    return sv;
}

String_View sv_right(String_View sv, size_t n) {
    if (sv.count == 0) return sv;

    if (sv.count < n) {
        sv.count = 0;
    }

    sv.count -= n;

    return sv;
}

char *sv_to_cstr(String_View sv) {
    char *result = malloc(sv.count + 1);

    memcpy(result, sv.items, sv.count);
    result[sv.count + 1] = '\0';

    return result;
}

#endif

#endif // SV_H
