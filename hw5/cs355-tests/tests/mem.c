#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include "mem.h"

long request_size;
void *region;
int m_error = INITIALIZE;
HEAD *all_memory;
HEAD *free_head;


int Mem_Init(long sizeOfRegion){
    
    if (sizeOfRegion <= 0 || all_memory != NULL) {
        m_error = E_BAD_ARGS;
        return FAIL;
    }
    
    long page_size = getpagesize();
    request_size = ((sizeOfRegion + sizeof(HEAD) + page_size - 1) / page_size) * page_size; // revist 
    region = mmap(NULL, request_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (region == MAP_FAILED) {
        return FAIL; 
    }

    region = memset(region, 0xaa, request_size);
    //printf("size req:%ld", request_size);
    all_memory = (HEAD*)region;
    all_memory->prev = NULL;
    all_memory->next = NULL;
    all_memory->next_free = NULL;
    all_memory->free = TRUE;       
    all_memory->size = request_size - HEADER_SIZE;

    free_head = all_memory;
    return SUCCESS;
}

long eight_byte_align(long x){
    return (x + ALIGN) & ~ALIGN;
}

void insert_node(HEAD **head, HEAD *node){
    
    HEAD *tmp = *head;
    if ( tmp == NULL|| tmp->size < node->size) {
        node->next_free = tmp;
        *head = node;
    }else{
        while (tmp->next_free != NULL && tmp->next_free->size > node->size) {
            tmp = tmp->next_free;
        }
        node->next_free = tmp->next_free;
        tmp->next_free = node;
    }
}

void remove_node(HEAD **head, HEAD *node){

    if (*head == node){
        *head = (*head)->next;
        return;
    }

    HEAD *tmp = *head;
    while (tmp->next_free != NULL && tmp->next_free != node){
        tmp = tmp->next_free; 
    }
    tmp->next_free = tmp->next_free->next_free;
}

void *fragment_memory(HEAD * memory_block, long size_requested){

    int leftover_size = memory_block->size - size_requested;
    int total_bytes = leftover_size / HEADER_SIZE;
    HEAD* frag = (HEAD *)(memory_block + total_bytes);
    frag->size = size_requested;
    frag->free = FALSE;
    frag->next_free = NULL;
    frag->prev = memory_block;
    frag->next = memory_block->next;
    memory_block->next = frag;
    memory_block->size = leftover_size;
    
    void *ptr = (void *)(frag + 1);
    return ptr;
}   

void *Mem_Alloc(long size){
    
    if (size <= 0 || all_memory == NULL) { 
        m_error = E_BAD_ARGS;
        return NULL;
    }
    uint32_t new_size = (uint32_t) (eight_byte_align(size) + HEADER_SIZE);
    HEAD *worstFit = free_head;
    void *ptr = NULL;

    if (worstFit==NULL || worstFit->size < new_size){
        m_error = E_NO_SPACE;
        return NULL;
    }
    int leftover_space = worstFit->size - new_size; 
    if (leftover_space >= MIN_SIZE){ // here
        ptr = fragment_memory(worstFit, new_size);
    } 
    else { // changed here
        worstFit->free = FALSE;
        remove_node(&free_head, worstFit);
        // remove this node from free list
        ptr = (void*)(worstFit + 1 );
    }

    return ptr;
}

//coalesce is 1, coalesce the entire list and wherever possible
int coalesce_list(HEAD **mem_block){

    if (*mem_block == NULL || (*mem_block)->next == NULL){
        return SUCCESS;
    }

    (*mem_block)->next_free = NULL;

    if ((*mem_block)->free == TRUE && (*mem_block)->next->free == TRUE){
        HEAD *tmp = (*mem_block)->next;
        (*mem_block)->size += (*mem_block)->next->size;
        (*mem_block)->next = (*mem_block)->next->next;
        void *header = (void *)tmp;

        return coalesce_list(mem_block);
        //printf("both are free\n");
    }

    if ((*mem_block)->free == TRUE){
        //(*mem_block)->next_free = NULL;
        insert_node(&free_head, *mem_block);
    }
    return coalesce_list( &((*mem_block)->next) );
    
}



//when coalesce is 2, coalesce a local neighberhood whose size is upto you
int coalesce_zone(HEAD **mem_block){

    if (*mem_block == NULL || (*mem_block)->free == FALSE){
        return SUCCESS;
    }

    if ((*mem_block)->prev->free == FALSE && (*mem_block)->next->free == FALSE ){
        return SUCCESS;
    }

    remove_node(&free_head, *mem_block);
    HEAD *zone_list = *mem_block;
    HEAD *start = *mem_block;
    HEAD *end = *mem_block;
    

    while (start->prev->free == TRUE){
        HEAD *tmp = start->prev;
        remove_node(&free_head, tmp);
        start = tmp->prev;
    }

    while (end->next->free == TRUE){
        HEAD *tmp = end->next;
        remove_node(&free_head, tmp);
        end = tmp->next;
    }

    uint32_t size = start->size; 
    HEAD *next = end->next;  
    HEAD *prev = start->prev; 
    HEAD *coalesced = start;

    while (start != end){
        HEAD *tmp = start->next;
        size += tmp->size;
        void *recase = (void *)start;
        start = tmp;
    }

    coalesced = (HEAD *)coalesced;
    coalesced->size = size;
    coalesced->next = next; 
    coalesced->prev = prev; 
    coalesced->free = TRUE; 

    return SUCCESS; 
}

// we cheat and don't free anything
int Mem_Free(void *ptr, int coalesce){

    HEAD *mem_block = NULL; 
    if (ptr != NULL){
        mem_block  = (HEAD *)ptr - 1;
        mem_block->free = TRUE;
        mem_block->next_free = NULL;
        insert_node(&free_head, mem_block);
    }
    switch (coalesce){
        case 0:
            return SUCCESS;

        case 1: 
            free_head = NULL;
            return coalesce_list(&all_memory);

        case 2: 
            return coalesce_zone(&mem_block);

        default:
            m_error = E_BAD_ARGS;
            return FAIL;
    }
    
    m_error = E_BAD_ARGS;
    return FAIL;
}

void Mem_Dump(){

    HEAD *tmp = all_memory;

    printf("\nDump:\n");
    printf("list all memory head address: %p\n", tmp);
    while(tmp != NULL){
        printf("total size: %d, free %d, next: %p\n", tmp->size, tmp->free, tmp->next);
        tmp = tmp->next;
    }
    printf("\n\n");

    tmp = free_head;
    printf("list free memory head address: %p\n", tmp);
    while(tmp != NULL){
        printf("total size: %d, free %d, next: %p\n", tmp->size, tmp->free, tmp->next);
        tmp = tmp->next_free;
    }
    printf("\n");
}
