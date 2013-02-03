#include "memchunk.h"
#include <stdlib.h>

struct chunk
{
	uint32_t start;
	uint32_t length;
	struct chunk *next;
};

struct chunk *free_list = (void *)0;
struct chunk *used = (void *)0;

uint32_t max_free = 0;

// Add a chunk to a list
static void chunk_add(uint32_t start, uint32_t length, struct chunk **list)
{
	struct chunk *c = (struct chunk *)malloc(sizeof(struct chunk));
	c->start = start;
	c->length = length;
	c->next = *list;
	*list = c;
}

// Return 1 if the chunk overlaps an entry in a list, otherwise 0
static int chunk_overlaps(uint32_t start, uint32_t length, struct chunk *list)
{
	while(list)
	{
		uint32_t l_start = list->start;
		uint32_t l_end = l_start + list->length;
		uint32_t end = start + length;

		if((start < l_end) && (end > l_start))
			return 1;

		list = list->next;
	}

	return 0;
}

// Return 1 if the chunk is wholly contained within an entry in the list, otherwise 0
static int chunk_contains(uint32_t start, uint32_t length, struct chunk *list)
{
	while(list)
	{
		uint32_t l_start = list->start;
		uint32_t l_end = l_start + list->length;
		uint32_t end = start + length;

		if((start >= l_start) && (end <= l_end))
			return 1;

		list = list->next;
	}

	return 0;
}

// Return 1 if can allocate, otherwise 0
static int chunk_can_allocate(uint32_t start, uint32_t length)
{
	if(!chunk_contains(start, length, free_list))
		return 0;
	if(chunk_overlaps(start, length, used))
		return 0;
	return 1;
}

void chunk_register_free(uint32_t start, uint32_t length)
{
	chunk_add(start, length, &free_list);

	if((start + length) > max_free)
		max_free = start + length;
}

uint32_t chunk_get_any_chunk(uint32_t length)
{
	uint32_t test_address = 0;
	while(test_address < max_free)
	{
		if(chunk_can_allocate(test_address, length))
		{
			chunk_add(test_address, length, &used);
			return test_address;
		}
		test_address += 0x1000;	// Returned page aligned chunks
	}
	
	return 0;
}

uint32_t chunk_get_chunk(uint32_t start, uint32_t length)
{
	if(chunk_can_allocate(start, length))
	{
		chunk_add(start, length, &used);
		return start;
	}
	return 0;
}


