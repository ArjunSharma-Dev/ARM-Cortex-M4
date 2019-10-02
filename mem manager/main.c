#include "mem_manager.h"

struct block_node *ptr;

//Flags
int init_heap;

int main()
{
    int ch, heap_bytes, chunk, alloc_bytes, node_num;

    //Initialization
    init_heap = 0;

    while(1)
    {
        printf("1. Initialize Heap Memory\n2. Allocate Memory\n3. Free Memory\n4. Memory Dump\n5. Memory Exit\n");
        printf("Enter your choice : ");
        scanf("%d", &ch);
        if (ch==1)
        {
            if (init_heap == 0)
            {
                init_heap = 1;
                printf("Enter number of bytes of Heap : ");
                scanf("%d", &heap_bytes);
                printf("Enter chunk size (16, 32, 64, 128) : ");
                scanf("%d", &chunk);
                mem_init(heap_bytes, chunk);
                printf("Memory Initialized\nAddress starting at %p\n\n", used_list);
            }
            else
            {
                printf("Heap already initialized\nKindly select other choice\n\n");
            }
        }
        else if (ch==2)
        {
            if (init_heap == 1)
            {
                printf("Enter number of bytes of memory to be allocated : ");
                scanf("%d", &alloc_bytes);
                ptr = mem_alloc(alloc_bytes);
                if (ptr == NULL)
                {
                    printf("\nAllocation of %d bytes failed\nInsufficient Heap Memory\n\n", alloc_bytes);
                }
                else
                {
                    printf("\nNode %d allocated at address %p\n", ptr->node, ptr);
                }
            }
            else
            {
                printf("Heap memory not initialized\n\n");
            }
        }
        else if (ch==3)
        {
            if (init_heap == 1)
            {
                printf("Enter the node to be freed : ");
                scanf("%d", &node_num);
                mem_free(node_num);
            }
            else
            {
                printf("Heap memory not initialized\n\n");
            }
        }
        else if (ch==4)
        {
            if (init_heap == 1)
            {
                mem_dump();
            }
            else
            {
                printf("Heap memory not initialized\n\n");
            }
        }
        else if (ch==5)
        {
            if (init_heap == 1)
            {
                mem_exit();
                printf("\nHeap memory deallocated\n");
            }
            else
            {
                printf("Heap memory not initialized\n\n");
            }
            exit(0);
        }
        else
        {
            printf("Invalid choice\nPlease select proper choice\n\n");
        }
    }
    return 0;
}
