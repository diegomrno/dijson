/**
 *   __
 *  /\ \  __    __
 *  \_\ \/\_\  /\_\    ____    ___     ___
 *  /'_` \/\ \ \/\ \  /',__\  / __`\ /' _ `\
 * /\ \L\ \ \ \ \ \ \/\__, `\/\ \L\ \/\ \/\ \
 * \ \___,_\ \_\_\ \ \/\____/\ \____/\ \_\ \_\
 *  \/__,_ /\/_/\ \_\ \/___/  \/___/  \/_/\/_/
 *             \ \____/
 *              \/___/
 *
 * dijson 1.0 -  Modern, safe, header-only & minimalist json utility
 *               written in C
 */

// Copyright (c) 2025 - Diego Moreno
// This software is licensed under the MIT license (see LICENSE.txt)

#ifndef DIJSON_H
#define DIJSON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DIJSON_NULL,
    DIJSON_BOOL,
    DIJSON_NUMBER,
    DIJSON_STRING,
    DIJSON_ARRAY,
    DIJSON_OBJECT
} dijson_type_t;

struct dijson_value_t;

typedef struct dijson_kv_pair_t {
    char* key;
    struct dijson_value_t* value;
} dijson_kv_pair_t;

typedef struct dijson_value_t {
    dijson_type_t type;
    union {
        bool bool_value;
        double number_value;
        char* string_value;
        struct {
            struct dijson_value_t** items;
            size_t count;
        } array;
        struct {
            dijson_kv_pair_t* items;
            size_t count;
        } object;
    } as;
} dijson_value_t;

// API
static dijson_value_t* dijson_parse(const char* json_str);
static dijson_value_t* dijson_parse_from_file(const char* filename);
static char* dijson_dump(const dijson_value_t* value);
static bool dijson_dump_to_file(const dijson_value_t* value, const char* filename);
static void dijson_free(dijson_value_t* value);

// Accessors
static inline dijson_value_t* dijson_get(const dijson_value_t* obj, const char* key) {
    if (!obj || obj->type != DIJSON_OBJECT) return NULL;
    for (size_t i = 0; i < obj->as.object.count; ++i)
        if (strcmp(obj->as.object.items[i].key, key) == 0)
            return obj->as.object.items[i].value;
    return NULL;
}

static inline dijson_value_t* dijson_get_index(const dijson_value_t* arr, size_t index) {
    if (!arr || arr->type != DIJSON_ARRAY || index >= arr->as.array.count) return NULL;
    return arr->as.array.items[index];
}

static inline const char* dijson_get_string(const dijson_value_t* val) {
    return (val && val->type == DIJSON_STRING) ? val->as.string_value : NULL;
}

static inline double dijson_get_number(const dijson_value_t* val) {
    return (val && val->type == DIJSON_NUMBER) ? val->as.number_value : 0.0;
}

static inline bool dijson_get_bool(const dijson_value_t* val) {
    return (val && val->type == DIJSON_BOOL) ? val->as.bool_value : false;
}

// Parser
static void skip_ws(const char** p) {
    while (**p && isspace((unsigned char)**p)) (*p)++;
}

static bool str_eq(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

static dijson_value_t* parse_value(const char** p);

static dijson_value_t* new_value(dijson_type_t type) {
    dijson_value_t* val = (dijson_value_t*)calloc(1, sizeof(dijson_value_t));
    val->type = type;
    return val;
}

static char* parse_string(const char** p) {
    (*p)++; // skip '"'
    const char* start = *p;
    while (**p && **p != '"') 
    {if (**p == '\\') (*p)++; // skip escape
        (*p)++;
    }
    size_t len = *p - start;
    char* out = (char*)malloc(len + 1);
    strncpy(out, start, len);
    out[len] = 0;
    (*p)++;
    return out;
}

static dijson_value_t* parse_number(const char** p) {
    char* end = NULL;
    double num = strtod(*p, &end);
    dijson_value_t* val = new_value(DIJSON_NUMBER);
    val->as.number_value = num;
    *p = end;
    return val;
}

static dijson_value_t* parse_array(const char** p) {
    (*p)++; // skip [
    dijson_value_t** items = NULL;
    size_t count = 0;
    skip_ws(p);
    if (**p == ']') { (*p)++; return new_value(DIJSON_ARRAY); }
    while (**p) {
        dijson_value_t* item = parse_value(p);
        if (!item) break;
        items = realloc(items, (count + 1) * sizeof(*items));
        items[count++] = item;
        skip_ws(p);
        if (**p == ']') { (*p)++; break; }
        if (**p == ',') (*p)++;
        skip_ws(p);
    }
    dijson_value_t* arr = new_value(DIJSON_ARRAY);
    arr->as.array.items = items;
    arr->as.array.count = count;
    return arr;
}

static dijson_value_t* parse_object(const char** p) {
    (*p)++; // skip {
    dijson_kv_pair_t* items = NULL;
    size_t count = 0;
    skip_ws(p);
    if (**p == '}') { (*p)++; return new_value(DIJSON_OBJECT); }
    while (**p) {
        skip_ws(p);
        if (**p != '"') break;
        char* key = parse_string(p);
        skip_ws(p);
        if (**p != ':') break;
        (*p)++;
        skip_ws(p);
        dijson_value_t* val = parse_value(p);
        if (!val) break;
        items = realloc(items, (count + 1) * sizeof(*items));
        items[count].key = key;
        items[count].value = val;
        count++;
        skip_ws(p);
        if (**p == '}') { (*p)++; break; }
        if (**p == ',') (*p)++;
    }
    dijson_value_t* obj = new_value(DIJSON_OBJECT);
    obj->as.object.items = items;
    obj->as.object.count = count;
    return obj;
}

static dijson_value_t* parse_value(const char** p) {
    skip_ws(p);
    if (**p == 'n' && strncmp(*p, "null", 4) == 0) {
        *p += 4;
        return new_value(DIJSON_NULL);
    } else if (**p == 't' && strncmp(*p, "true", 4) == 0) {
        *p += 4;
        dijson_value_t* val = new_value(DIJSON_BOOL);
        val->as.bool_value = true;
        return val;
    } else if (**p == 'f' && strncmp(*p, "false", 5) == 0) {
        *p += 5;
        dijson_value_t* val = new_value(DIJSON_BOOL);
        val->as.bool_value = false;
        return val;
    } else if (**p == '"') {
        dijson_value_t* val = new_value(DIJSON_STRING);
        val->as.string_value = parse_string(p);
        return val;
    } else if (**p == '[') {
        return parse_array(p);
    } else if (**p == '{') {
        return parse_object(p);
    } else if (isdigit(**p) || **p == '-') {
        return parse_number(p);
    }
    return NULL;
}

static dijson_value_t* dijson_parse(const char* json_str) {
    const char* p = json_str;
    return parse_value(&p);
}

static dijson_value_t* dijson_parse_from_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    char* buffer = (char*)malloc(size + 1);
    fread(buffer, 1, size, f);
    buffer[size] = 0;
    fclose(f);
    dijson_value_t* val = dijson_parse(buffer);
    free(buffer);
    return val;
}

// Dumper
static void dump_value(const dijson_value_t* val, FILE* out);

static void dump_string(const char* s, FILE* out) {
    fputc('"', out);
    while (*s) {
        if (*s == '"') fputs("\"", out);
        else if (*s == '\\') fputs("\\\\", out);
        else fputc(*s, out);
        s++;
    }
    fputc('"', out);
}

static void dump_value(const dijson_value_t* val, FILE* out) {
    if (!val) return;
    switch (val->type) {
        case DIJSON_NULL: fputs("null", out); break;
        case DIJSON_BOOL: fputs(val->as.bool_value ? "true" : "false", out); break;
        case DIJSON_NUMBER: fprintf(out, "%g", val->as.number_value); break;
        case DIJSON_STRING: dump_string(val->as.string_value, out); break;
        case DIJSON_ARRAY:
            fputc('[', out);
            for (size_t i = 0; i < val->as.array.count; ++i) {
                if (i) fputc(',', out);
                dump_value(val->as.array.items[i], out);
            }
            fputc(']', out);
            break;
        case DIJSON_OBJECT:
            fputc('{', out);
            for (size_t i = 0; i < val->as.object.count; ++i) {
                if (i) fputc(',', out);
                dump_string(val->as.object.items[i].key, out);
                fputc(':', out);
                dump_value(val->as.object.items[i].value, out);
            }
            fputc('}', out);
            break;
    }
}

static char* dijson_dump(const dijson_value_t* value) {
    FILE* tmp = tmpfile();
    dump_value(value, tmp);
    fseek(tmp, 0, SEEK_END);
    size_t len = ftell(tmp);
    rewind(tmp);
    char* out = (char*)malloc(len + 1);
    fread(out, 1, len, tmp);
    out[len] = 0;
    fclose(tmp);
    return out;
}

static bool dijson_dump_to_file(const dijson_value_t* value, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return false;
    dump_value(value, f);
    fclose(f);
    return true;
}

// Memory cleanup
static void dijson_free(dijson_value_t* value) {
    if (!value) return;
    switch (value->type) {
        case DIJSON_STRING:
            free(value->as.string_value);
            break;
        case DIJSON_ARRAY:
            for (size_t i = 0; i < value->as.array.count; ++i)
                dijson_free(value->as.array.items[i]);
            free(value->as.array.items);
            break;
        case DIJSON_OBJECT:
            for (size_t i = 0; i < value->as.object.count; ++i) {
                free(value->as.object.items[i].key);
                dijson_free(value->as.object.items[i].value);
            }
            free(value->as.object.items);
            break;
        default: break;
    }
    free(value);
}

#ifdef __cplusplus
}
#endif

#endif // DIJSON_H
