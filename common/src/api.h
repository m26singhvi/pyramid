#ifndef __API_H__
#define __API_H__

typedef enum api_id {
    FIND_MAX = 0,

    MAX_API /*Keep this always last*/
} api_id_t;

typedef enum API_STATUS_ {
    API_FAILURE = 0,
    API_SUCCESS,
    API_INVALID_INPUT,
    API_INVALID_ID,
} api_status_t;

api_status_t main_api(void *input, void *output, api_id_t id); 
typedef api_status_t (*api_func)(void *, void *);
typedef struct api_ {
    api_func api;
}api;

typedef struct int_vector {
    int *vector;
    long long len;
} int_vector_t;
#endif /* __API_H__ */
