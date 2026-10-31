#include <stdio.h>
#include <stdlib.h>
#include "allocator.h"

int main(){ 

    void *a  = my_malloc(100);
    my_free(a);
    stress_test();
    advanced_testing_suite();
    return 0;
}