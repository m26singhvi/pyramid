#include "api.h"
#include<assert.h>
#include<stdio.h>

int main()
{
    char *in = "test_data.txt";
    char *max = "test_data_op.txt";
    assert(main_api(in, max,FIND_MAX) == API_SUCCESS);
    return 0;
}
        

