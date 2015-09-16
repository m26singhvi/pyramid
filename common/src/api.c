#include "api.h"

api_status_t find_max(void * , void*);

api api_list[] = 
{
    {find_max}
};


api_status_t main_api(void *input, void *output, api_id_t id) 
{

    if(id < MAX_API){
        return api_list[id].api(input,output);
    }
    return API_INVALID_ID;
}

api_status_t find_max(void *input, void *output) 
{
    int_vector_t *in = input;
    int max_int = in->vector[0]; /*Assume first element is max*/
    for(long long i = 1; i < in->len ; i++){
        if(max_int < in->vector[i]) {
            max_int = in->vector[i];
        }
    }
    *((int *)output) = max_int;
    return API_SUCCESS;
}
