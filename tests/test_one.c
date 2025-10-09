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

size_t write_compressed(char *path_obj_sha, char *path_tmp_file)
{
    FILE *source = fopen(path_tmp_file, "rb");
    FILE *dest = fopen(path_obj_sha, "rb");
    if (source == NULL || dest == NULL) {
        perror("Error when opening file in write_compress.");
        return 1;
    }

    int ret, flush;
    unsigned have;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) return 1;

    size_t read = 0;

    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void) deflateEnd(&strm);
            return 1;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);

            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have) {
                (void)deflateEnd(&strm);
                return 1;
            }
            read += have;
        } while (strm.avail_out == 0);

        assert(strm.avail_in == 0);
    } while (flush != Z_FINISH);

    assert(ret == Z_STREAM_END);
    (void) deflateEnd(&strm);
    return read;
}

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

unsigned char *get_sha_encryption_file_d(char *path_obj)
{
    struct stat st = {0};
    if (stat(path_obj, &st) != 0) {
        perror("Not a valid path to a file.");
        return NULL;
    }

    FILE *source = fopen(path_obj, "rb");
    if (source == NULL) {
        perror("Error when opening file.");
        return NULL;
    }
    unsigned char sha1_buff[EVP_MAX_MD_SIZE];
    unsigned int read;

    int status = sha1_on_file(source, sha1_buff, &read);
    if (status != 0) {
        perror("Failed to get the sha1 encryption.");
        fclose(source);
        return NULL;
    }

    size_t hex_len = (size_t)read * 2 + 1;
    unsigned char *sha_obj = malloc(hex_len);
    if (sha_obj == NULL) {
        perror("Allocation error.");
        return NULL;
    }
    for (int i = 0; i < read; i++) {
        sprintf((char *)sha_obj + i*2, "%02x", sha1_buff[i]);
    }

    sha_obj[hex_len-1] = '\0';
    fclose(source);

    return sha_obj;
}

unsigned char *get_header_blob(char *path_obj) 
{
    size_t size = file_size(path_obj);
    unsigned char *size_str = itos_d(size);
    unsigned char frmt[] = "blob";

    size_t size_frmt = strlen((char *)frmt);
    size_t size_len = strlen((char *)size_str);
    size_t size_header = size_frmt + size_len + 2; // '\0' and 0x20
    // size_t size_obj = size_header + size;
    unsigned char *header = malloc(size_header);
    if (header == NULL) {
        perror("Allocation error.");
        return NULL;
    }
    unsigned char *current = header;

    memcpy(current, frmt, size_frmt);
    current += size_frmt;
    *current = ' ';
    current += 1;

    memcpy(current, size_str, size_len);
    current += size_len;
    *current = '\0';
    if (current - header == size_header) {
        perror("Error in the header.");
        return NULL;
    }
    return header;
}

int copy_tmp_file_blob(unsigned char *header, char *path_tmp_file, char *path_obj) 
{
    FILE *tmp_file = fopen(path_tmp_file, "rb");
    FILE *source = fopen(path_obj, "rb");
    if (tmp_file == NULL || source == NULL) {
        perror("Error when opening file in serialize blob.");
        return -1;
    }
    int status = fwrite(header, 1, sizeof(header), tmp_file);
    if (status == 0) {
        perror("Error when transfering header.");
        return -1;
    }

    int total = 0;
    unsigned char file_chunk[CHUNK];
    while (true) {
        if (status == 0)
            break;
        status = fread(file_chunk, 1, sizeof(file_chunk), source);
        if (fwrite(file_chunk, 1, status, tmp_file) != status) {
            perror("Error during the copy.");
            return -1;
        }
        total += status;
    }
    return total;
}

int serialize_blob(char *path_obj) 
{
    char *cwd = NULL;
    if ((cwd = getcwd(NULL, 0)) == NULL) {
        perror("Could not find the current working dir.");
        return 1;
    }
    char *repo = repo_find_f(cwd);
    free(cwd);

    struct stat st = {0};
    unsigned char *sha_obj = get_sha_encryption_file_d(path_obj);
    if (sha_obj == NULL) 
        return 1;

    unsigned char *header = get_header_blob(path_obj);
    if (header == NULL) 
        return 1;
    
    char *path_tmp_file = join_path_d(repo, "objects/tmp/tmp_blob_file");
    int status = copy_tmp_file_blob(header, path_tmp_file, path_obj);
    if (status == -1) {
        free(sha_obj);
        free(header);
        free(path_tmp_file);
        return 1;
    }

    unsigned char current_obj_dir[3]; // first two character of the SHA1 digest plus the 0x00
    memcpy(current_obj_dir, sha_obj, 2);
    current_obj_dir[2] = 0x00;
    char *path_objects = join_path_d(repo, "objects/");
    char *path_dir_sha = join_path_d(path_objects, current_obj_dir);
    if (stat(path_dir_sha, &st) != 0) 
        mkdir(path_dir_sha, PERMISSION);
    
    unsigned char *blob_obj = sha_obj + 2;
    char *path_blob_obj = join_path_d(path_dir_sha, (char *)blob_obj);
    int read = write_compressed(path_blob_obj, path_tmp_file);
    free(path_blob_obj);
    if (read == 1) {
        perror("Error when compressing.");
        free(sha_obj);
        free(header);
        free(path_tmp_file);
        return 1;
    }

    remove(path_tmp_file);
    free(sha_obj);
    free(header);
    free(path_tmp_file);

    return 0;
}

int main() {
    char *path = "/home/marshallmallow/Folders/Personnal Project/Write yourself a git/Wyag/tests/test_one.c";
    serialize_blob(path);

    return 0;
}

