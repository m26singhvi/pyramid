#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
void * get_function_ptr(char * func_name,void **lib_handle  ){
    char *error;
    void *ret;

    *lib_handle = dlopen("../dynamic/bin/common_lib.so",RTLD_NOW|RTLD_GLOBAL);
    if (!*lib_handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror();    /* Clear any existing error */

    ret = dlsym(*lib_handle, func_name);

    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        dlclose(*lib_handle);
        exit(EXIT_FAILURE);
    }

    return ret;
}

