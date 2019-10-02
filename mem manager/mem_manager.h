#include <stdio.h>
#include <stdlib.h>

struct block_node {
    int node;
    struct block_node *next;
    int chunks;
    char data[0];
};

static int node_val=1;
int chunk_size, total_size;
int final_address;
struct block_node *used_list, *free_list, *newptr;

void mem_init (int heap_bytes, int chunk)
{
    int temp;
    chunk_size = chunk;
    //size_temp = (heap_bytes + 2*sizeof(*newptr));
    //temp = (size_temp)%chunk_size;
    temp = heap_bytes%chunk_size;
    if (temp==0)
    {
        //used_list = malloc(size_temp);
        used_list = malloc(heap_bytes + 2*sizeof(*newptr));
        total_size = heap_bytes;
        used_list->chunks = 0;
    }
    else
    {
        //used_list = malloc(size_temp + chunk_size - temp);
        used_list = malloc(heap_bytes + 2*sizeof(*newptr) + chunk_size - temp);
        total_size = heap_bytes + chunk_size - temp;
        used_list->chunks = 0;
    }
    final_address = (int)used_list + total_size + 2*sizeof(*newptr);
    used_list->next = NULL;
    //free_list = used_list + sizeof(*newptr);
    free_list = used_list+1;
    free_list->chunks = total_size/chunk_size;
    //free_list->next = used_list + 2*sizeof(*newptr);
    free_list->next = free_list+1;
    free_list->next->node = 0;
    free_list->next->chunks = (final_address - (int)free_list->next)/chunk_size;
    free_list->next->next = NULL;
}

void *mem_alloc (int alloc_bytes)
{
    int byte_alloc, temp, hole=0, flag=0;
    struct block_node *temp_ptr, *ptr_free;
    //Calculate memory bytes to be allocated to node
    temp = (alloc_bytes + sizeof(*newptr))%chunk_size;
    if (temp==0)
    {
        byte_alloc = alloc_bytes + sizeof(*newptr);
    }
    else
    {
        byte_alloc = alloc_bytes + sizeof(*newptr) + chunk_size - temp;
    }

    temp_ptr = free_list->next;
    while(temp_ptr != NULL)
    {
        //Allocation of memory
        if (byte_alloc <= temp_ptr->chunks*chunk_size)
        {
            /****************************            Case 1: Fresh Heap             ******************************/
            if (used_list->next == NULL)
            {
                used_list->next = temp_ptr;
                newptr = temp_ptr;
                newptr->node = node_val++;
                newptr->chunks = byte_alloc/chunk_size;
                newptr->next = NULL;
                used_list->chunks = used_list->chunks + byte_alloc/chunk_size;
                temp_ptr = (int)newptr + byte_alloc;
                free_list->chunks = free_list->chunks - byte_alloc/chunk_size;
                if ((int)temp_ptr == final_address)
                {
                    temp_ptr = NULL;
                }
                else
                {
                    temp_ptr->node = 0;
                    temp_ptr->chunks = (final_address - (int)temp_ptr)/chunk_size;
                    temp_ptr->next = NULL;
                }
                free_list->next = temp_ptr;
                return newptr;
            }
            else
            {
                //Traverse the used_list to find the appropriate hole or free space where node should be allocated
                newptr = used_list;
                while(newptr->next != NULL)
                {
                    if (newptr->next > temp_ptr)
                    {
                        hole = 1;
                        break;
                    }
                    newptr = newptr->next;
                }
                //Common code
                newptr->next = temp_ptr;
                newptr = newptr->next;

                /****************************  Case 2: No hole, ie, free space at end of used_list ******************************/
                if (hole == 0)
                {
                    newptr->node = node_val++;
                    newptr->chunks = byte_alloc/chunk_size;
                    newptr->next = NULL;
                    used_list->chunks = used_list->chunks + byte_alloc/chunk_size;
                    temp_ptr = (int)newptr + byte_alloc;
                    free_list->chunks = free_list->chunks - byte_alloc/chunk_size;
                    if ((int)temp_ptr == final_address)
                    {
                        temp_ptr = NULL;
                    }
                    else
                    {
                        temp_ptr->node = 0;
                        temp_ptr->chunks = (final_address - (int)temp_ptr)/chunk_size;
                        temp_ptr->next = NULL;
                    }
                }
                /*******************************         Case 3: Hole present          **********************************/
                else
                {
                    //When hole is split into allocated node and another hole
                    if (byte_alloc < temp_ptr->chunks*chunk_size)
                    {
                        temp_ptr = (int)newptr + byte_alloc;
                        temp_ptr->node = 0;
                        temp_ptr->next = newptr->next;
                        temp_ptr->chunks = newptr->chunks - byte_alloc/chunk_size;
                        newptr->next = (int)newptr + newptr->chunks*chunk_size;
                        newptr->node = node_val++;
                        newptr->chunks = byte_alloc/chunk_size;
                        used_list->chunks = used_list->chunks + byte_alloc/chunk_size;
                        free_list->chunks = free_list->chunks - byte_alloc/chunk_size;
                    }
                    //When hole is completely allocated by node
                    else
                    {
                        temp_ptr = temp_ptr->next;
                        newptr->next = (int)newptr + byte_alloc;
                        newptr->node = node_val++;
                        newptr->chunks = byte_alloc/chunk_size;
                        used_list->chunks = used_list->chunks + byte_alloc/chunk_size;
                        free_list->chunks = free_list->chunks - byte_alloc/chunk_size;
                    }
                }

                //Common code for updation of free_list and returning newptr
                if (flag == 0)
                {
                    free_list->next = temp_ptr;
                }
                else
                {
                    ptr_free = free_list->next;
                    while(ptr_free->next != newptr)
                    {
                        ptr_free = ptr_free->next;
                    }
                    ptr_free->next = temp_ptr;
                }
                return newptr;
            }
        }
        //Traverse the free_list to find appropriate address to allocate node
        else
        {
            flag = 1;
            temp_ptr = temp_ptr->next;
        }
    }
    //Allocation of memory failed
    return temp_ptr;
}

void fragmentation()
{
    int temp, flag=0;
    newptr = free_list->next;

    while(newptr->next != NULL)
    {
        temp = (int)newptr + newptr->chunks*chunk_size;
        if ((int)newptr->next == temp)
        {
            printf("\nBlock at address %p merged into block at address %p\n", newptr->next, newptr);
            newptr->node = 0;
            newptr->chunks += newptr->next->chunks;
            newptr->next = newptr->next->next;
            flag = 1;
        }
        else
        {
            newptr = newptr->next;
        }
    }
    if (flag == 0)
    {
        printf("\nNo fragmentation\n");
    }
}

void mem_free (int node_num)
{
    int flag = 0;
    struct block_node *temp_ptr;
    newptr = used_list->next;
    while(newptr!=NULL)
    {
        //If node to be freed is detected
        if (node_num == newptr->node)
        {
            //Updating used_list
            if (newptr == used_list->next)
            {
                used_list->next = newptr->next;
            }
            else
            {
                temp_ptr = used_list->next;
                while(temp_ptr->next != newptr)
                {
                    temp_ptr = temp_ptr->next;
                }
                temp_ptr->next = newptr->next;
            }
            //Updating free_list
            if (free_list->next == NULL)
            {
                newptr->next = NULL;
                free_list->next = newptr;
            }
            else if (newptr < free_list->next)
            {
                newptr->next = free_list->next;
                free_list->next = newptr;
            }
            else
            {
                temp_ptr = free_list->next;
                while(temp_ptr->next < newptr)
                {
                    temp_ptr = temp_ptr->next;
                }
                newptr->next = temp_ptr->next;
                temp_ptr->next = newptr;
            }
            //Updating free size and used size
            free_list->chunks = free_list->chunks + newptr->chunks;
            used_list->chunks = used_list->chunks - newptr->chunks;
            flag = 2;
            break;
        }
        else
        {
            newptr = newptr->next;
        }
        flag = 1;
    }
    //If used_list is empty
    if (flag == 0)
    {
        printf ("\nUsed list is empty\nMemory freeing of Node failed\n");
    }
    //If node to be freed was not detected
    else if (flag == 1)
    {
        printf ("\nNode %d is not used\nMemory freeing of Node failed\n", node_num);
    }
    //Successful freeing of node
    else if (flag == 2)
    {
        printf("\nMemory freed\n");
        //Check for fragmentation
        fragmentation();
    }
    return;
}

void mem_dump ()
{
    printf("\nUSED LIST\t\t\tUsed Size = %d",used_list->chunks);
    printf("\n*******************************************************");
    printf("\n*   Node       |       Address       |   Size(Chunk)  *");
    printf("\n*******************************************************\n");
    newptr = used_list->next;
    while(newptr != NULL)
    {
        printf("%d\t\t%p\t\t%d\n", newptr->node, newptr, newptr->chunks);
        newptr = newptr->next;
    }

    printf("\n\n\nFREE LIST\t\t\tFree Size = %d",free_list->chunks);
    printf("\n*******************************************************");
    printf("\n*   Node       |       Address       |   Size(Chunk)  *");
    printf("\n*******************************************************\n");
    newptr = free_list->next;
    while(newptr != NULL)
    {
        printf("%d\t\t%p\t\t%d\n", newptr->node, newptr, newptr->chunks);
        newptr = newptr->next;
    }

    return;
}

void mem_exit ()
{
    free(used_list);
}

