#ifndef _DYNAMIC_LIB_INTERFACE_H__
#define _DYNAMIC_LIB_INTERFACE_H___


void * get_function_ptr(char * func_name);

#define ASSIGN_FUNC_PTR(func_name, func_ptr) \
    *(void **)(&func_ptr) = get_function_ptr(func_name); 

#endif
