#include<stdio.h>
#include<inttypes.h>
#include<errno.h>
#include<stdlib.h>
#include "api.h"

api_status_t find_max(void * , void*);

api api_list[] = 
{
    {find_max}
};

int lines_input_file(char *input){
    FILE *file = fopen( (char*)input, "r" );
    char line[125] = {};
    long num=0;
    if(file != NULL){
        while(fgets ( line, sizeof line, file ) != NULL){
            num++;
        }
    fclose(file);
    }
    return num;
}


api_status_t read_input_file(char *input , int_vector_t * output){
    FILE *file = fopen( (char*)input, "r" );
    char line[125] = {};
    int num;
    long long count = 0;
    if(file == NULL){
        return API_INVALID_INPUT;
    }
    while(fgets ( line, sizeof line, file ) != NULL){
        num = strtoumax(line, NULL, 10);
        if (errno == ERANGE) {
            /* Could not convert. */
            return API_INVALID_INPUT;
        }
        output->vector[count++] = num;
    }
    fclose(file);
    return API_SUCCESS;
}

api_status_t write_op_file(int_vector_t *res, char *output) 
{
    FILE *f = fopen(output, "w");

    if (f == NULL)
    {
        return API_INVALID_INPUT;
    }

    for(long long i = 0 ; i < res->len ; i++) {
        fprintf(f, "%d\n", res->vector[i]);
    }
    fclose(f);

    return API_SUCCESS;
}
api_status_t main_api(void *input, void *output, api_id_t id) 
{

    if(id < MAX_API){
        return api_list[id].api(input,output);
    }
    return API_INVALID_ID;
}

api_status_t find_max(void *input, void *output) 
{
    int max =0;
    int_vector_t vec ;
    vec.len = lines_input_file(input);
    vec.vector = malloc(vec.len * sizeof(int));
    if(read_input_file(input, &vec) != API_SUCCESS){
        return API_INVALID_INPUT;
    }
    if(vec.len > 0){
        max = vec.vector[0];
    } else {
        return API_INVALID_INPUT;
    }
    for(long long i = 1 ; i < vec.len ; i++) {
        if(max < vec.vector[i]) {
            max = vec.vector[i];
        }
    }

    int_vector_t result ={&max, 1};
    if(write_op_file(&result , output) == API_SUCCESS) {
        free(vec.vector);
        return API_SUCCESS;
    }
    free(vec.vector);
    return API_INVALID_INPUT;
}

