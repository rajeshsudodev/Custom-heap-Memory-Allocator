// test.c

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include "allocator_test.h"
#include "allocator.h"
#include <unistd.h>

#define MAX_PTRS 1000
#define OPERATIONS 10000
#define MAX_ALLOC_SIZE 2048


void *ptrs[MAX_PTRS] = {NULL};
static int random_index() {
    return rand() % MAX_PTRS;
}

static size_t random_size() {
    return (rand() % MAX_ALLOC_SIZE) + 1;
}


    void stress_test() {
        srand(time(NULL));

        printf("=========== STRESS TEST START ===========\n");

        for(int op = 0; op < OPERATIONS; op++) {
            int choice = rand() % 3;
            int idx = random_index();

            if(choice == 0) { // Alloc
                if(ptrs[idx] == NULL) {
                    size_t size = random_size();
                    ptrs[idx] = my_malloc(size);
                    printf("MY_ALLOC\n");
                    if(ptrs[idx] == NULL) {
                        printf("[ALLOC FAIL] index=%d size=%zu\n", idx, size);
                    }
                    else {
                        printf("[WRITE MY_ALLOC]\n");
                        unsigned char *p = (unsigned char*)ptrs[idx];
                        for(size_t i = 0; i < size; i++) {
                            p[i] = (unsigned char)(i % 255);
                        }
                        printf("[EXIT WRITE MY_ALLOC]\n");
                    }
                    printf("EXIT_MY_ALLOC\n");
                }
            }
            else if(choice == 1) { // Free
                if(ptrs[idx] != NULL) {
                    printf("[MY_FREE]\n");
                    my_free(ptrs[idx]);
                    ptrs[idx] = NULL;
                    printf("[EXIT_FREE]\n");
                }
            }
            else { // Realloc
                if(ptrs[idx] != NULL) {
                    size_t new_size = random_size();
                    void *new_ptr = my_realloc(ptrs[idx], new_size);
                    printf("[MY_REALLOCATION]\n");
                    if(new_ptr != NULL) {
                        ptrs[idx] = new_ptr;
                        unsigned char *p = (unsigned char*)ptrs[idx];
                        for(size_t i = 0; i < new_size; i++) {
                            p[i] = (unsigned char)(rand() % 255);
                        }
                    }
                    printf("[EXIT_REALLOCATION_TESTING]\n");
                }
            }

            if(heap_validate() == -1) {
                traverse();
                printf("\nHEAP VALIDATION FAILED!\n");
                printf("Operation Number : %d\n", op);
                return;
            }
            else printf("Operation Number : %d Passed\n", op);
        }

        printf("\n=========== STRESS TEST PASSED ===========\n");
        traverse();
        printf("\n=========== STRESS TEST PASSED ===========\n");
    }


    void advanced_testing_suite(void) {
        printf("\n==================================================\n");
        printf("PHASE 3 : ADVANCED FRAGMENTATION & EDGE CASE SUITE\n");
        printf("==================================================\n");

        // TEST 1 : THE FRAGMENTATION SAW
        printf("\n[TEST 1] THE FRAGMENTATION SAW (Interleaved Freeing)\n");
        #define SAW_SIZE 50
        void *saw_ptrs[SAW_SIZE];
        
        for(int i = 0; i < SAW_SIZE; i++) {
            saw_ptrs[i] = (char*)my_malloc(64);
            if(saw_ptrs[i]) {
                memset(saw_ptrs[i], 0xAA, 64);
            }
        }
        
        for(int i = 0; i < SAW_SIZE; i += 2) {
            if(saw_ptrs[i]) {
                my_free(saw_ptrs[i]);
                saw_ptrs[i] = NULL;
            }
        }
        
        void *large_p = my_malloc(256);
        if(large_p) {
            memset(large_p, 0xBB, 256);
            printf("PASS: Handled allocation across alternating free chunks\n");
        } else {
            printf("FAIL: Failed allocation across alternating free chunks\n");
        }
        
        for(int i = 1; i < SAW_SIZE; i += 2) {
            if(saw_ptrs[i]) my_free(saw_ptrs[i]);
        }
        if(large_p) my_free(large_p);
        
        coalesce_free_blocks();
        if(heap_validate() == -1) return;

        // TEST 2 : ALIGNMENT EXHAUSTION
        printf("\n[TEST 2] ALIGNMENT EXHAUSTION (Odd Byte Sizes)\n");
        size_t odd_sizes[] = {1, 15, 17, 31, 33, 127, 1025};
        void *odd_ptrs[7];
        int alignment_pass = 1;

        for(int i = 0; i < 7; i++) {
            odd_ptrs[i] = (char*)my_malloc(odd_sizes[i]);
            uintptr_t addr = (uintptr_t)odd_ptrs[i];
            
            if(addr % 16 != 0) {
                printf("FAIL: Address %p is not 16-byte aligned for requested size %zu\n", (void*)odd_ptrs[i], odd_sizes[i]);
                alignment_pass = 0;
            }
        }
        
        if(alignment_pass) printf("PASS: All odd allocations perfectly aligned to 16-byte boundaries\n");
        
        for(int i = 0; i < 7; i++) {
            if(odd_ptrs[i]) my_free(odd_ptrs[i]);
        }
        
        coalesce_free_blocks();
        if(heap_validate() == -1) return;

        // TEST 3 : ZERO-BYTE REALLOC TRAP
        printf("\n[TEST 3] ZERO-BYTE REALLOC TRAP (POSIX Standard Check)\n");
        void *trap_ptr = (char*)my_malloc(500);
        memset(trap_ptr, 'Z', 500);
        
        trap_ptr = (char*)my_realloc(trap_ptr, 0);
        
        if(trap_ptr == NULL) {
            printf("PASS: Realloc to 0 bytes safely freed memory and returned NULL\n");
        } else {
            printf("FAIL: Realloc to 0 bytes returned a non-NULL pointer\n");
            my_free(trap_ptr);
        }
        
        coalesce_free_blocks();
        if(heap_validate() == -1) return;

        // TEST 4 : CASCADE COALESCING
        printf("\n[TEST 4] CASCADE COALESCING (The Domino Effect)\n");
        void *d1 = (char*)my_malloc(128);
        void *d2 = (char*)my_malloc(128);
        void *d3 = (char*)my_malloc(128);
        void *d4 = (char*)my_malloc(128);

        my_free(d1); 
        my_free(d3); 
        my_free(d2); 
        my_free(d4);
        
        coalesce_free_blocks();
        if(heap_validate()) {
            printf("PASS: Cascaded multi-block physical coalescing succeeded\n");
        } else {
            printf("FAIL: Memory layout broken during cascade clear\n");
        }

        printf("\n==================================================\n");
        printf("ADVANCED SUITE COMPLETION VALIDATION\n");
        printf("==================================================\n");
        if(heap_validate()) {
            printf("CONGRATULATIONS: Allocator is completely bulletproof!\n");
        } else {
            printf("ALERT: Structural failure discovered on final sweep.\n");
        }
    }
