#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#define PERMISSION 0755
#define INIT "init"
#define SHA_LENGTH 21
#define COMPRESSED_CHUNK 128
#define CHUNK 1024
#define SHA_CHUNK 8192 // 8KB

enum OBJECT_FORMAT { COMMIT = 0, TREE = 1, TAG = 2, BLOB = 3, NO_FORMAT = -1 };
typedef struct Object {
    char sha[SHA_LENGTH];
    enum OBJECT_FORMAT format;
    size_t size;
} Object;

#endif