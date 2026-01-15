#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <unistd.h>
#include <sys/time.h>

const char help_message[] = "Usage: sxor [options] file1.dat file2.txt ...\n"
                            "  -e to encode\n"
                            "  -d to decode\n"
                            "  -k the key to use for the encode or decode (max length: 4096)\n";

/*
    file structure
    4 bytes - filename size
    variable size - original filename
    blob - filedata
*/

#define BUFFER_SIZE (256 * 1024)

void xor(uint8_t* key, size_t key_len, uint8_t* data, size_t len, uint8_t* out){
    uint64_t* data64 = (uint64_t*)data;
    uint64_t* key64 = (uint64_t*)key;
    uint64_t* out64 = (uint64_t*)out;

    size_t itr = len / sizeof(uint64_t);
    size_t rem = len % sizeof(uint64_t);
    size_t i, r;

    for(i = 0;i < itr;i++){
        out64[i] = data64[i] ^ key64[0];
    }

    uint8_t key_index = 0;
    for(r = (len - rem);r < len;r++){
        out[r] = data[r] ^ key[(key_index = key_index % 8)];
        key_index++;
    }
}

void run_conversion(FILE* read_f, FILE* write_f, uint8_t* key){
    size_t key_len = strlen(key);
    int rfd = fileno(read_f);
    int wfd = fileno(write_f);
    uint8_t buf[BUFFER_SIZE];
    uint8_t out[BUFFER_SIZE];
    ssize_t buf_sz;
    do{
        buf_sz = read(rfd, buf, BUFFER_SIZE);
        xor(key, key_len, buf, buf_sz, out);
        write(wfd, out, buf_sz);
    }while(buf_sz != 0);
    fclose(read_f);
    fclose(write_f);
}

int encode_file(char* fname, uint8_t* key){
    FILE* read_f = fopen(fname, "rb");
    if(read_f == NULL){
        fprintf(stderr, "Failed to open file for reading\n");
        return 1;
    }

    char ofname[64];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(time(NULL));
    sprintf(ofname, "%ld%ld%ld.dat", tv.tv_sec, tv.tv_usec, rand());

    FILE* write_f = fopen(ofname, "wb");
    if(write_f == NULL){
        fprintf(stderr, "Failed to open file for writing\n");
        return 1;
    }

    uint32_t name_len = (uint32_t)strlen(fname);
    uint8_t* buf = (uint8_t*)malloc(1024);
    uint8_t* out = (uint8_t*)malloc(1024);

    memcpy(buf, &name_len, sizeof(uint32_t));
    xor(key, strlen(key), buf, sizeof(uint32_t), out);
    fwrite(out, 1, sizeof(uint32_t), write_f);

    buf = (uint8_t*)realloc(buf, name_len);
    out = (uint8_t*)realloc(out, name_len);

    memcpy(buf, fname, name_len);
    xor(key, strlen(key), buf, name_len, out);
    fwrite(out, 1, name_len, write_f);

    free(buf);
    free(out);
    fflush(write_f);

    run_conversion(read_f, write_f, key);
}

int decode_file(char* fname, char* key){
    FILE* read_f = fopen(fname, "rb");
    if(read_f == NULL){
        fprintf(stderr, "Failed to open file for reading\n");
        return 1;
    }
 
    uint32_t name_len = 0;
    uint8_t* buf = (uint8_t*)malloc(1024);
    uint8_t* out = (uint8_t*)malloc(1024);
    
    fread(buf, 1, sizeof(uint32_t), read_f);
    xor(key, strlen(key), buf, sizeof(uint32_t), out);
    memcpy(&name_len, out, sizeof(uint32_t));

    buf = (uint8_t*)realloc(buf, name_len + 1);
    out = (uint8_t*)realloc(out, name_len + 1);

    fread(buf, 1, name_len, read_f);
    xor(key, strlen(key), buf, name_len, out);
    out[name_len] = 0x00;

    FILE* write_f = fopen(out, "wb");
    if(write_f == NULL){
        fprintf(stderr, "Failed to open file for writing\n");
        return 1;
    }

    free(buf);
    free(out);
    fflush(read_f);

    run_conversion(read_f, write_f, key);
}

int main(int argc, char* argv[]){
    int encode = 0, decode = 0;
    char *key = NULL;
    int arg;
    while((arg = getopt(argc, argv, "edk:")) != -1){
        switch(arg){
            case 'e':
                encode = 1;
                break;
            case 'd':
                decode = 1;
                break;
            case 'k':
                key = optarg;
                break;
            default:
                fprintf(stderr, help_message);
                return -1;
        }
    }

    if(!encode && !decode){
        fprintf(stderr, "Select atleast one operation -e (encode) or -d (decode)\n");
        fprintf(stderr, help_message);
        return -1;
    }

    if(key == NULL){
        fprintf(stderr, "No key argument passed\n");
        fprintf(stderr, help_message);
        return -1;
    }

    if(strlen(key) != 8){
    //if(strlen(key) > 4096){
        fprintf(stderr, "Key must be 8 characters\n");
        fprintf(stderr, help_message);
        return -1;
    }

    if(optind < argc){
        while(optind < argc){
            if(encode){
                encode_file(argv[optind++], (uint8_t*)key);
            }else{
                decode_file(argv[optind++], (uint8_t*)key);
            }
        }
    }
}