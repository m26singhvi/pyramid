#include "api.h"
#include<assert.h>

int main()
{
    int v[] = {1,3,344,5,67};
    int_vector_t in;
    in.vector = v;
    in.len = 5;
    int max = 0;
    assert(main_api(&in, &max,FIND_MAX) == API_SUCCESS);
    assert(max == 344);
    return 0;
}
        

