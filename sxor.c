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

#define BUFFER_SIZE 1024

/* It can be binary, i hate dealing with signed char */
uint8_t* xor(uint8_t* key, size_t key_len, uint8_t* data, size_t len){
    uint64_t key_index = 0;
    for(int i = 0;i < len;i++){
        data[i] ^= key[(key_index = key_index % key_len)];
        key_index++;
    }
    return data;
}

void run_conversion(FILE* read_f, FILE* write_f, uint8_t* key){
    uint8_t buf[BUFFER_SIZE];
    size_t buf_sz;
    do{
        buf_sz = fread(buf, 1, BUFFER_SIZE, read_f);
        fwrite(xor(key, strlen((char*)key), buf, buf_sz), 1, buf_sz, write_f);
    }while(buf_sz != 0);
    fclose(read_f);
    fclose(write_f);
}

int encode_file(char* fname, char* key){
    FILE* read_f = fopen(fname, "rb");
    if(read_f == NULL){
        fprintf(stderr, "Failed to open file for reading\n");
        return 1;
    }

    char buf[1024], ofname[64];

    /* Hey it works */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(time(NULL));
    sprintf(ofname, "%ld%ld%ld.dat", tv.tv_sec, tv.tv_usec, rand());

    FILE* write_f = fopen(ofname, "wb");
    if(write_f == NULL){
        fprintf(stderr, "Failed to open file for writing\n");
        return 1;
    }

    uint32_t fname_sz = (uint32_t)strlen(fname);
    memcpy(buf, &fname_sz, sizeof(fname_sz));
    fwrite(xor((uint8_t*)key, strlen(key), (uint8_t*)buf, sizeof(fname_sz)), 1, sizeof(fname_sz), write_f);
    memcpy(&buf, fname, fname_sz);
    fwrite(xor((uint8_t*)key, strlen(key), (uint8_t*)buf, fname_sz), 1, fname_sz, write_f);

    run_conversion(read_f, write_f, key);
}

int decode_file(char* fname, char* key){
    FILE* read_f = fopen(fname, "rb");
    if(read_f == NULL){
        fprintf(stderr, "Failed to open file for reading\n");
        return 1;
    }

    uint8_t buf[1024];
    char ofname[64];

    uint32_t name_len;
    fread(buf, 1, sizeof(uint32_t), read_f);
    memcpy(&name_len, xor((uint8_t*)key, strlen(key), buf, sizeof(uint32_t)), sizeof(uint32_t));
    fread(ofname, 1, name_len, read_f);
    xor((uint8_t*)key, strlen(key), ofname, name_len);
    ofname[name_len] = '\0';

    FILE* write_f = fopen(ofname, "wb");
    if(write_f == NULL){
        fprintf(stderr, "Failed to open file for writing\n");
        return 1;
    }

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

    if(strlen(key) > 4096){
        fprintf(stderr, "Key is too large\n");
        fprintf(stderr, help_message);
        return -1;
    }

    if(optind < argc){
        while(optind < argc){
            if(encode){
                encode_file(argv[optind++], key);
            }else{
                decode_file(argv[optind++], key);
            }
        }
    }
}