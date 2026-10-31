// debug.c

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include "allocator.h"
#include "allocator_test.h"
#define CANARY 0xDEC0DE

/**
 * validation return conversion:
 * 1 -> validation successful
 * -1-> canary validation failed 
 */
int validate(Header_block *head){

    Header_block *curr = head;
    while(curr ){
        if(curr->canary != CANARY) {
            printf("Error !!!\n");

            return -1;
        }
        curr = curr->next;
    }

    return 1;
}

void  traverse_list(Header_block *head_node){

    Header_block *curr = head_node;

    while(curr){
        printf("size of header       : %zu \n", sizeof(Header_block));
        printf("size of footer       : %zu \n", sizeof(Footer_block));
        printf("size of header->size : %zu \n", sizeof(curr->size));
        printf("size of header->free : %zu \n", sizeof(curr->free));
        printf("size of header->next : %zu \n", sizeof(curr->next));
        printf("size of header->canary : %zu \n", sizeof(curr->canary));
        
        printf("Block: %p, Size: %zu, Free: %d\n", (void*)curr, curr->size, curr->free);
        curr = curr->next;
    }
}