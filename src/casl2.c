#include "compiler.h"

FILE *out_fp;

int init_output(char *filename){
    char *str;
    sprintf(str,"%s.csl",filename);
    out_fp = fopen(str,"w");
    if(out_fp == NULL) {
        return -1;
    } else {
        printf("[INFO]Make \"%s\".\n",str);
        return 0;
    }
}