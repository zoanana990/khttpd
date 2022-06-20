#include "mime.h"

extern hashtable_t *MIME;
#ifdef __KERNEL__
char *mime_strdup(char *str)
{
    size_t len;
    char *ret;

    len = strlen(str);
    ret = MALLOC(len + 1);
    if (__builtin_expect(ret != NULL, 1))
        memcpy(ret, str, len + 1);

    FREE(str);

    return ret;
}
#endif
hashtable_t *mime_init(size_t size)
{
    hashtable_t *MIME_ = NULL;
    int i = 0;

MALLOC_HASH_TABLE:
    /* Create hashtable */
    MIME_ = MALLOC(sizeof(hashtable_t));
    if (MIME_ == NULL) {
        PRINT("HASH TABLE MALLOC FAIL\n");
        goto MALLOC_HASH_TABLE;
    }

MALLOC_MIME_TABLE:
    /* Create mimetable */
    MIME_->mimetable = MALLOC(sizeof(mime_t *) * size);
    if (MIME_->mimetable == NULL) {
        PRINT("MIME TABLE MALLOC FAIL\n");
        goto MALLOC_MIME_TABLE;
    }

    for (; i < size; i++)
        MIME_->mimetable[i] = NULL;

    MIME_->size = size;


    return MIME_;
}

mime_t *mime_alloc(char *file_extension, char *http_type)
{
    mime_t *pair;

MALLOC_MIME_NODE:
    pair = MALLOC(sizeof(mime_t));
    if (pair == NULL) {
        PRINT("MIME NODE MALLOC FAIL\n");
        goto MALLOC_MIME_NODE;
    }
DUP_FILE_EXTENSION:
    pair->file_extension = mime_strdup(file_extension);
    if (pair->file_extension == NULL) {
        PRINT("DUP FILE EXTENSION FAIL\n");
        goto DUP_FILE_EXTENSION;
    }
DUP_HTTP_TYPE:
    pair->http_type = mime_strdup(http_type);
    if (pair->http_type == NULL) {
        PRINT("DUP HTTP TYPE FAIL\n");
        goto DUP_HTTP_TYPE;
    }
    pair->next = NULL;

    return pair;
}

int mime_hash(hashtable_t *hashtable, char *file_extension)
{
    unsigned long int value = 0x61C88647;
    // PRINT("%ld\n", value);
    size_t n = strlen(file_extension);

    int i = 0;

    /* Hash function */
    for (; value < ULONG_MAX && i < n; i++) {
        value = value << 8;
        value += file_extension[i];
    }

    return value % hashtable->size;
}

void mime_insert(hashtable_t *hashtable, char *file_extension, char *http_type)
{
    int index = 0;

    mime_t *newpair = NULL, *next = NULL, *last = NULL;

    index = mime_hash(hashtable, file_extension);

    next = hashtable->mimetable[index];

    /* if next is exist*/
    while (next != NULL && next->file_extension != NULL &&
           strcmp(file_extension, next->file_extension) > 0) {
        last = next;
        next = next->next;
    }
    if (next != NULL && next->file_extension != NULL &&
        strcmp(file_extension, next->file_extension) == 0) {
        /* Already a pair */
        FREE(next->http_type);
        next->http_type = mime_strdup(http_type);

    } else {
        /* insert a new pair to linked list, allocate memory space */
        newpair = mime_alloc(file_extension, http_type);
        if (next == hashtable->mimetable[index]) {
            /* Insert to the head of Linked list */
            newpair->next = next;
            hashtable->mimetable[index] = newpair;
        } else if (!next) {
            /* Insert to the tail of linked list */
            last->next = newpair;
        } else {
            /* Insert a node in the middle */
            newpair->next = next;
            last->next = newpair;
        }
    }
}
char *get_mime_type(hashtable_t *hashtable, char *file_extension)
{
    int index = 0;
    mime_t *pair;
    char *ext = strchr(file_extension, '.');

    index = mime_hash(hashtable, ext);

    /* MIME linked list traversal */
    pair = hashtable->mimetable[index];
    if (!pair)
        return "text/plain";

    while (pair->file_extension != NULL &&
           strcmp(ext, pair->file_extension) > 0) {
        pair = pair->next;
    }

    if (pair->file_extension == NULL ||
        strcmp(ext, pair->file_extension) != 0) {
        return "text/plain";
    } else {
        return pair->http_type;
    }
}

void mime_free(hashtable_t *hashtable)
{
    mime_t *curr, *tmp;

    int i = 0;

    for (; i < hashtable->size; i++) {
        curr = hashtable->mimetable[i];
        while (curr) {
            FREE(curr->file_extension);
            FREE(curr->http_type);
            tmp = curr;
            curr = curr->next ? curr->next : NULL;
            FREE(tmp);
        }
    }
    FREE(hashtable);
}

void mime_create(size_t size)
{
    MIME = mime_init(size);
    mime_insert(MIME, ".ez", "application/andrew-inset");
    mime_insert(MIME, ".aw", "application/applixware");
    mime_insert(MIME, ".atom", "application/atom+xml");
    mime_insert(MIME, ".atomcat", "application/atomcat+xml");
    mime_insert(MIME, ".atomsvc", "application/atomsvc+xml");
    mime_insert(MIME, ".ccxml", "application/ccxml+xml");
    mime_insert(MIME, ".cu", "application/cu-seeme");
    mime_insert(MIME, ".davmount", "application/davmount+xml");
    mime_insert(MIME, ".ecma", "application/ecmascript");
    mime_insert(MIME, ".emma", "application/emma+xml");
    mime_insert(MIME, ".epub", "application/epub+zip");
    mime_insert(MIME, ".pfr", "application/font-tdpfr");
    mime_insert(MIME, ".gz", "application/gzip");
    mime_insert(MIME, ".tgz", "application/gzip");
    mime_insert(MIME, ".stk", "application/hyperstudio");
    mime_insert(MIME, ".jar", "application/java-archive");
    mime_insert(MIME, ".ser", "application/java-serialized-object");
    mime_insert(MIME, ".class", "application/java-vm");
    mime_insert(MIME, ".json", "application/json");
    mime_insert(MIME, ".lostxml", "application/lost+xml");
    mime_insert(MIME, ".hqx", "application/mac-binhex40");
    mime_insert(MIME, ".cpt", "application/mac-compactpro");
    mime_insert(MIME, ".mrc", "application/marc");
    mime_insert(MIME, ".ma", "application/mathematica");
    mime_insert(MIME, ".mb", "application/mathematica");
    mime_insert(MIME, ".nb", "application/mathematica");
    mime_insert(MIME, ".mathml", "application/mathml+xml");
    mime_insert(MIME, ".mml", "application/mathml+xml");
    mime_insert(MIME, ".mbox", "application/mbox");
    mime_insert(MIME, ".mscml", "application/mediaservercontrol+xml");
    mime_insert(MIME, ".mp4s", "application/mp4");
    mime_insert(MIME, ".doc", "application/msword");
    mime_insert(MIME, ".dot", "application/msword");
    mime_insert(MIME, ".mxf", "application/mxf");
    mime_insert(MIME, ".a", "application/octet-stream");
    mime_insert(MIME, ".bin", "application/octet-stream");
    mime_insert(MIME, ".bpk", "application/octet-stream");
    mime_insert(MIME, ".o", "application/octet-stream");
    mime_insert(MIME, ".obj", "application/octet-stream");
    mime_insert(MIME, ".pkg", "application/octet-stream");
    mime_insert(MIME, ".so", "application/octet-stream");
    mime_insert(MIME, ".iso", "application/octet-stream");
    mime_insert(MIME, ".dmg", "application/octet-stream");
    mime_insert(MIME, ".elc", "application/octet-stream");
    mime_insert(MIME, ".oda", "application/oda");
    mime_insert(MIME, ".opf", "application/oebps-package+xml");
    mime_insert(MIME, ".ogx", "application/ogg");
    mime_insert(MIME, ".onepkg", "application/onenote");
    mime_insert(MIME, ".xer", "application/patch-ops-error+xml");
    mime_insert(MIME, ".pdf", "application/pdf");
    mime_insert(MIME, ".pgp", "application/pgp-encrypted");
    mime_insert(MIME, ".pki", "application/pkixcmp");
    mime_insert(MIME, ".xml", "application/xml");
    mime_insert(MIME, ".tar", "application/x-tar");
    mime_insert(MIME, ".sh", "application/x-shellscript");
    mime_insert(MIME, ".pyc", "application/x-python-code");
    mime_insert(MIME, ".rpa", "application/x-redhat-package-manager");
    mime_insert(MIME, ".bmp", "image/bmp");
    mime_insert(MIME, ".cgm", "image/cgm");
    mime_insert(MIME, ".gif", "image/gif");
    mime_insert(MIME, ".jpe", "image/jpeg");
    mime_insert(MIME, ".jpeg", "image/jpeg");
    mime_insert(MIME, ".jpg", "image/jpeg");
    mime_insert(MIME, ".pjpg", "image/jpeg");
    mime_insert(MIME, ".png", "image/png");
    mime_insert(MIME, ".ras", "image/x-cmu-raster");
    mime_insert(MIME, ".c", "text/x-c");
    mime_insert(MIME, ".cc", "text/x-c");
    mime_insert(MIME, ".h", "text/x-c");
    mime_insert(MIME, ".cpp", "text/x-c");
    PRINT("%s", "MIME TABLE CREATE SUCCESSFUL\n");
}

// #ifndef __KERNEL__
// int main()
// {
//     mime_create(128);

//     PRINT("Insert Sucessful\n");

//     char *file1 = ".modules.order.cmd";
//     char *file2 = "http_server.c";

//     // char *request_type = strchr(file1, '.');
//     PRINT("%s\n", get_mime_type(MIME, file1));
//     // PRINT("%s\n", strchr(file2, '.'));

//     mime_free(MIME);
//     return 0;
// }
// #endif