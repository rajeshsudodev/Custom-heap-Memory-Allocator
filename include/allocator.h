#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

// ARCHITECTURE DESING
// Header 
typedef struct Header_block{
    size_t size ;     // 8 B
    int free ;        // 4 B
    uint32_t canary;  // 4 B
    struct Header_block *next; // 8 B  pointer usually consumes 8 Bytes from Memory     
}Header_block;  // header size totally 24 B % 8 == 0;

// Footer
typedef struct Footer_block{
    size_t size;    // 8
    int free ;      // 4
    uint32_t canary;// 4
}Footer_block;  // footer size totally 16 B % 8 == 0;

// Mode 
typedef enum mode{
    MY_ALLOC,
    REALLOC,
    EXTEND,
    SHRINK
}mode;

extern Header_block *head_node;
extern Header_block *tail_node ;

// Function that return PAYLOAD pointer to user 
void *my_malloc(size_t size);
void *my_realloc(void *ptr, size_t size);

// non - return functions
void my_free(void *ptr);
void coalesce_free_blocks();

void *find_free(Header_block *head_node, size_t size);
void Stamp_metadata(Header_block *header, size_t size , int free);

Header_block *split_up_realloc(Header_block *head, size_t req_size, mode REALLOC);
Header_block *split_up_malloc(Header_block *head, size_t req_size, mode MY_ALLOC);

Header_block *split_used_block(Header_block *head ,size_t req_size);
Header_block *Merge_next_if_fit(Header_block *header, size_t needed_size);
Header_block *merge_next_adjacent(Header_block *header);

void stress_test() ;

void traverse_list(Header_block *head_node);
static inline void traverse(){
    traverse_list(head_node);
}
int validate(Header_block *head);

static inline int heap_validate(){
    return validate(head_node);
}
void advanced_testing_suite(void);

#endif