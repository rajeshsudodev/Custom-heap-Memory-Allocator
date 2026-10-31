// allocator.c

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "allocator.h"
#include <time.h>

#define METADATA (sizeof(Header_block) + sizeof(Footer_block))
#define MIN_SPLIT_SIZE (METADATA + 16)
#define ALIGNMENT 8
#define CANARY 0xDEC0DE
#define MAX_PTRS 1000
#define OPERATIONS 10000
#define MAX_ALLOC_SIZE 2048

Header_block *head_node = NULL;
Header_block *tail_node = NULL;

uint16_t my_alloc_count = 0;
uint16_t coalesce_count = 0;
uint16_t my_free_count = 0;
uint16_t split_up_count = 0;
uint16_t reuse_count = 0;
uint16_t Double_free_count = 0;
uint8_t Invalid_free_attempts= 0;

size_t total_req_size = 0;
size_t Total_allocated_size = 0;
size_t Max_allocated_size = 0;
size_t Total_freed_size = 0;



size_t align(size_t size){
    // size = (size + (ALIGNMENT - 1)) & (~ (ALIGNMENT -1));
    size = ALIGNMENT + ( size & ~(ALIGNMENT-1));
    return size;
}

// scan heap list for available free 
void *find_free(Header_block *head_node, size_t req_size){
    // Guard
    if(head_node == NULL) return NULL;
    Header_block *current = head_node;
    
    // Scan 
    while(current != NULL){

        if(current->size >=req_size && current->free == 1){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// stamping Metadata at header and footer 
void Stamp_metadata(Header_block *header, size_t size , int free){

    // header stamp
    header->size = size;
    Footer_block *footer = (Footer_block*) ( (unsigned char*)header + sizeof(Header_block) + header->size);
    
    footer->size = size;
    
    
    header->free = footer->free = free;
    header->canary = footer->canary = CANARY;

    header->next = NULL;
    
    // footer stamp
    

    return ;
}

Header_block *split_used_block(Header_block *head ,size_t req_size){
    
    // header init
    Header_block *header = head;
    // calculate the remaining size 
    size_t remaining = header->size - req_size;

    // Guards
    if(header == NULL) return NULL;
    if(req_size == 0) return NULL;
    if(remaining < MIN_SPLIT_SIZE) return NULL;

    // curve new free block  , end point / used break point
    Header_block *new_free_block = (Header_block*)( (unsigned char*)header + METADATA + req_size);

    new_free_block->next = header->next;
    header->next = new_free_block;

    Stamp_metadata(new_free_block, remaining -METADATA, 1);

    // List Surgery
    if(header == tail_node){
        tail_node = new_free_block;
        tail_node->next = NULL;
    }

    header->size = req_size;
    return new_free_block;

}

Header_block *split_up_realloc(Header_block *head, size_t req_size, mode REALLOC){
    
    return split_used_block(head, req_size);
}

// split_up_malloc
Header_block *split_up_malloc(Header_block *head, size_t req_size, mode MY_ALLOC){
    return split_used_block(head, req_size);
}

Header_block *merge_next_adjacent(Header_block *header){

    Header_block *next = header->next;
    size_t merged_size = header->size + next->size + METADATA;

    if(next == tail_node){
        tail_node  = header;
    }
    Stamp_metadata(header, merged_size , header->free);

    header->next = next->next;
    return header;
}

Header_block *Merge_next_if_fit(Header_block *head, size_t needed_size){

    if(head->free && head->next->free){

        size_t  combined_payload = head->size + head->next->size + METADATA;
        if(combined_payload >= needed_size){
            return merge_next_adjacent(head);
        }
    }
    return NULL;
}

// NOTE : user passed the payload pointer 
void *my_malloc(size_t req_size){
    // Guards
    if(req_size <= 0) return NULL;
    
    req_size = align(req_size);
    size_t total_size = req_size + METADATA;

    // check if any free found 
    void *free_found = (void*)find_free(head_node,req_size);
    Header_block *header = NULL;

    // if sacn not found any free block 
    if(free_found == NULL){

        // create new block and add in the list 
        header = (Header_block*) sbrk(total_size);

        // sys call success 
        if(header != (void*)-1){
            
            // List Surgery 
            if(head_node == NULL){
                head_node = tail_node = header;
                tail_node->next = NULL;
            }
            else {
                tail_node->next = header;
                tail_node = header;
                tail_node->next = NULL;
            }
            
            Stamp_metadata(header, req_size, 0);
            
            // successful operation
            // return (unsigned char*)header + sizeof(Header_block);
        }
            // operation fails
        else return NULL ;
    }

    // if free found in list 
    else
    {
        header = free_found;
        
        // split block 
        Header_block *splitted_block  = (Header_block *)split_up_malloc(header, req_size, MY_ALLOC);


        // split success
        if(splitted_block != NULL){
            split_up_count++;
        }
        // update current resuse block state flags
        Stamp_metadata(header, header->size, 0);   // header->size was update in split_up function
    }

    void *payload = (unsigned char*)header + sizeof(Header_block);
    return payload;
    
}

// NOTE : user passed the payload pointer 
void *my_realloc(void *ptr, size_t resize){
    // Totally 3 paths , SHRINK, EXTEND , FALLBACK
    // user passes payload pointer

    // GUARDS
    if(resize < 0u) return NULL;
    if(ptr == NULL) return my_malloc(resize);
    if(resize == 0){
        my_free(ptr);
        return NULL;
    }
    
    resize = align(resize);

    // move one step back to get Header
    Header_block *header = (Header_block*)( (unsigned char*)ptr - sizeof(Header_block));
    size_t old_size = header->size;
    if(old_size == resize) {
        return ptr;
    }
    // SHRINK
    if(old_size > resize){

        // slit used block 
        Header_block *splitted_block = split_up_realloc(header, resize, REALLOC);

        if(splitted_block!= NULL) split_up_count++;


        Stamp_metadata(header, header->size, 0); // header->size was update in split_up function

        return (unsigned char*)header + sizeof(Header_block);
        
    }

    // EXPAND IN-PLACE
    if(header->next && header->next->free){

        // size_t needed_size = resize - header->size;
        Header_block *merged = Merge_next_if_fit(header, resize);

        if(merged != NULL){
            header = merged;

            Header_block *splitted_block = (Header_block *)split_up_realloc(merged, resize, EXTEND);
            if(splitted_block != NULL) split_up_count++;

            Stamp_metadata(header, header->size, 0);

            return (unsigned char*)header +  sizeof(Header_block);
        }

    }
    
    // FALLBACK MOMENT
    void *new_payload = my_malloc(resize);
    if(new_payload == NULL) return NULL;

    size_t copy_size = (old_size < resize) ? old_size : resize;
    memcpy(new_payload, ptr, copy_size);
    // void *payload = (unsigned char*) header +sizeof(Header_block);
    
    // my_free(user payload)
    my_free(ptr);

    return new_payload;
}

void my_free(void *ptr){

    // user function
    Header_block *header = (Header_block*) ( ( unsigned char *) ptr - sizeof(Header_block));
    Footer_block *footer  = (Footer_block*) ( (unsigned char* ) header + sizeof(Header_block) + header->size);

    if(header->canary != CANARY || footer->canary != CANARY){
        printf("HEAP CORRUPTION DETECTED\n");
        return ;
    }

    if(header->free) {
        Double_free_count++;
        printf("DOUBLE FREE DETECTED \n");
    }

    header->free = footer->free = 1;
    my_free_count ++;

    Total_freed_size += header->size;

    printf("Freed block %p (%zu bytes)\n", (void*)header, header->size);
    printf("[EXITED MY_FREE]\n");

    return ;
}

void coalesce_free_blocks(){
    Header_block *current  = head_node;

    while(current != NULL && current->next != NULL){

        if(current->free && current->next->free){
            current = merge_next_adjacent(current);
            continue;
        }
        current = current->next;
    }
    
}

