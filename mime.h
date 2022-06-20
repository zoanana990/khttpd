#ifndef _MIME_H_
#define _MIME_H_

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#define MALLOC(size) vmalloc(size)
#define FREE(x) vfree(x)
#define PRINT printk
#else
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MALLOC(size) malloc(size)
#define FREE(ptr) free(ptr)
#define PRINT printf
#define mime_strdup strdup
#endif

/**
 * A hashtable contain a pointer array which point to mime_t
 * due to a pointer array, we need a pointer to pointer,
 * that is **mimetable
 * +---------+
 * | *mime_t |
 * +---------+
 * | *mime_t |
 * +---------+
 * |   ...   |
 * +---------+
 *
 * and in each mime_t, there is
 * +-----------------+
 * | *file_extension |
 * +-----------------+
 * | *http_type      |
 * +-----------------+
 * | *next           |
 * +-----------------+
 *
 * the mime_t * is a linked list
 */
typedef struct mime {
    char *file_extension;
    char *http_type;
    struct mime *next;
} mime_t;

typedef struct hashtable_s {
    size_t size;
    mime_t **mimetable;
} hashtable_t;

hashtable_t *mime_init(size_t size);
void mime_create(size_t size);
void mime_free(hashtable_t *hashtable);
char *get_mime_type(hashtable_t *hashtable, char *file_extension);
void mime_insert(hashtable_t *hashtable, char *file_extension, char *http_type);
int mime_hash(hashtable_t *hashtable, char *file_extension);
mime_t *mime_alloc(char *file_extension, char *http_type);
/* Initialize MIME type */
extern hashtable_t *MIME;

#endif