#include "../include/main.h"
#include "../include/init.h"
#include "../include/cat_file.h"
#include "../include/hash_object.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

size_t file_size(char *file_path)
{
    FILE *pfile = fopen(file_path, "rb");
    if (pfile == NULL) {
        return (size_t)-1;
    }
    
    if (fseek(pfile, 0, SEEK_END) != 0) {
        fclose(pfile);
        return (size_t)-1;
    }
    
    long size = ftell(pfile);
    if (size == -1L) {
        fclose(pfile);
        return (size_t)-1;
    }
    
    fclose(pfile);
    return (size_t)size;
}

unsigned char *itos_d(size_t size)
{
    size_t nbytes = snprintf(NULL, 0, "%ld", size) + 1;
    unsigned char *str = (unsigned char *)malloc(nbytes);
    snprintf((char *)str, nbytes, "%ld", size);
    return str;
}

int sha1_on_file(FILE *file, unsigned char *out, unsigned int *read)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL)
        return -1;
    
    const EVP_MD *md = EVP_sha1();
    if (EVP_DigestInit_ex(mdctx, md, NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        return -1;
    }

    unsigned char *buff[SHA_CHUNK];
    size_t len;

    while ((len = fread(buff, 1, SHA_CHUNK, file)) > 0) {
        if (EVP_DigestUpdate(mdctx, buff, len) != 1) {
            EVP_MD_CTX_free(mdctx);
            return -1;
        }
    }

    unsigned int outlen;
    if (EVP_DigestFinal_ex(mdctx, out, &outlen) != 1) {
        EVP_MD_CTX_free(mdctx);
        return -1;
    }

    *read = outlen;

    EVP_MD_CTX_free(mdctx);
    return 0;
}

void serialize_blob(char *path_obj) 
{
    struct stat st = {0};
    if (stat(path_obj, &st) != 0) {
        perror("Not a valid path to a file.");
        return;
    }

    FILE *source = fopen(path_obj, "rb");
    unsigned char sha1_buff[EVP_MAX_MD_SIZE];
    unsigned int read;

    int status = sha1_on_file(source, sha1_buff, &read);
    if (status != 0) {
        perror("Failed to get the sha1 encryption.");
        return;
    }

    size_t hex_len = (size_t)read * 2 + 1;
    unsigned char sha_obj[hex_len];
    for (int i = 0; i < read; i++) {
        sprintf((char *)sha_obj + i*2, "%02x", sha1_buff[i]);
    }

    sha_obj[hex_len-1] = '\0';
    printf("%s\n", sha_obj);


    // unsigned char buffer[size];
    // size_t size_deflate = 0;
    // size_deflate = write_compressed_d(source, buffer);
    // fclose(source);

    size_t size = file_size(path_obj);
    unsigned char *size_str = itos_d(size);
    unsigned char frmt[] = "blob";

    size_t size_frmt = strlen((char *)frmt);
    size_t size_len = strlen((char *)size_str);
    size_t size_header = size_frmt + size_len + 2; // '\0' and 0x20
    // size_t size_obj = size_header + size;
    unsigned char *header = malloc(size_header);
    unsigned char *current = header;


    memcpy(current, frmt, size_frmt);
    current += size_frmt;
    *current = ' ';
    current += 1;

    memcpy(current, size_str, size_len);
    current += size_len;
    *current = '\0';
    printf("%s", header);
    if (current - header == size_header) {
        perror("Error in the header.");
        return;
    }
    free(header);
    free(size_str);
}

int main() {
    char *path = "/home/marshallmallow/Folders/Personnal Project/Write yourself a git/Wyag/tests/test_one.c";
    serialize_blob(path);

    return 0;
}

