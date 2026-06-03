#include <stdio.h>
#include <stdlib.h>
#include "allocator.h"

int main(){ 

    void *a  = my_malloc(100);
    my_free(a);
    advanced_testing_suite();
    stress_test();
    return 0;
}