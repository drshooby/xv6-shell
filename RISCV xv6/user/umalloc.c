#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "../kernel/param.h"

#define PAGE_SZ 4096
#define MIN_BLOCK_SZ 48
#define FIRST_FIT 0
#define BEST_FIT 1
#define WORST_FIT 2

short fsm = 0;
// used to indicate whether a NULL return is safe or 
bool safe = true;

struct mem_block {
    char name[8];
    uint64 size;
    struct mem_block *next_block;
    struct mem_block *prev_block;
} __attribute__((packed));

struct free_block {
  struct mem_block block_header;
  struct mem_block *next_free;
  struct mem_block *prev_free;
} __attribute__((packed));

struct mem_block *block_list_head;
struct mem_block *block_list_tail;
struct free_block *free_list_head;
struct free_block *free_list_tail;

void
add_block(struct mem_block *block)
{
  if (block_list_head == NULL) {
    block_list_head = block;
    block_list_tail = block;
    block->next_block = NULL;
    block->prev_block = NULL;
  } else {
    block_list_tail->next_block = block;
    block->prev_block = block_list_tail;
    block->next_block = NULL;
    block_list_tail = block;
  }
}

void
add_free(struct mem_block *block) 
{
  struct free_block *free = (struct free_block *) block;
  if (free_list_head == NULL) {
    free_list_head = free;
    free_list_tail = free;
    free->next_free = NULL;
    free->prev_free = NULL;
  } else {
    free_list_tail->next_free = (struct mem_block *) free;
    free->prev_free = (struct mem_block *) free_list_tail;
    free->next_free = NULL;
    free_list_tail = free;
  }
}

void
remove_free(struct free_block *remove)
{
  if (free_list_head == NULL || remove == NULL) {
    return;
  }

  if (free_list_head == remove) {
    free_list_head = (struct free_block *) remove->next_free;
  }

  if (free_list_tail == remove) {
    free_list_tail = (struct free_block *) remove->prev_free;
  }

  if (remove->next_free != NULL) {
    struct free_block *tmp = (struct free_block *) remove->next_free;
    tmp->prev_free = remove->prev_free;
  }

  if (remove->prev_free != NULL) {
    struct free_block *tmp = (struct free_block *) remove->prev_free;
    tmp->next_free = remove->next_free;
  }
}

void
set_free(struct mem_block *b)
{
  // last bit is always 1
  b->size |= 0x01;
}

bool
is_free(struct mem_block *b)
{
  return (b->size & 0x01) == 0x01;
}

void
set_used(struct mem_block *b)
{
  // last bit is toggled between 0 and 1
  b->size ^= 0x01;
}

uint64
real_size(uint size)
{
  return size & ~(0x01);
}

uint64
align(uint64 orig_size, uint alignment)
{
  uint64 new_size = (orig_size / alignment) * alignment;
  if (orig_size % alignment != 0) {
    new_size += alignment;
  }
  return new_size;
}

void
insert_split(struct mem_block *block, struct mem_block *split)
{
  struct mem_block *tmp = block->next_block;
  block->next_block = split;
  split->prev_block = block;
  split->next_block = tmp;
  // if there is a next block, update its previous block
  if (tmp != NULL) {
    tmp->prev_block = split;
  }
}

void*
split(struct mem_block *block, uint64 block_size, uint nbytes)
{
  // create new block
  struct mem_block *new_block_used = (struct mem_block *) (((char *) block) + block_size - nbytes);
  strcpy(new_block_used->name, "split");
  new_block_used->size = nbytes;
  new_block_used->next_block = NULL;
  new_block_used->prev_block = NULL;

  // update old block
  block->size = block_size - nbytes;
  if (!is_free(block)) {
    set_free(block);
  }

  // add new block to block list
  insert_split(block, new_block_used);
  return new_block_used;
}

void*
check_can_split(struct mem_block *block, uint block_size, uint nbytes)
{
  safe = true;
  uint64 total_size_needed = align(sizeof(struct mem_block) + nbytes, 16);
  // block is NULL, this should never happen but just in case
  if (block == NULL) {
    safe = false;
    fprintf(2, "problem: broken implementation - free block is NULL\n");
    return NULL;
  }
  // can't split, not enough space for new block
  if (block_size - total_size_needed < MIN_BLOCK_SZ) {
    safe = false;
    return NULL;
  }
  // if block is the same size as the total size needed, we don't need to split
  if (block_size == total_size_needed) {
    return NULL;
  }

  return split(block, block_size, total_size_needed);
}

void*
first_fit(uint nbytes)
{  
  struct free_block *free = free_list_head;
  while (free != NULL) {
    uint64 block_size = free->block_header.size;
    if (block_size >= nbytes) {
      // try to create new block
      struct mem_block *res = check_can_split((struct mem_block *) free, block_size, nbytes);
      // either return the split block or the whole block
      if (res != NULL) {
        return res;
      } else {
        if (!safe) {
          free = (struct free_block *) free->next_free;
          continue;
        }
        set_used((struct mem_block *) free);
        remove_free(free);
        return (struct mem_block *) free;
      }
    }
    free = (struct free_block *) free->next_free;
  }
  return NULL;
}

void*
best_fit(uint nbytes)
{
  // find the smallest fitting block
  struct free_block *free = free_list_head;
  struct free_block *min = NULL;
  while (free != NULL) {
    if (free->block_header.size >= nbytes) {
      if (min == NULL || min->block_header.size > free->block_header.size) {
        min = free;
      }
    }
    free = (struct free_block *) free->next_free;
  }
  if (min == NULL) {
    return NULL;
  }
  struct mem_block *ret = check_can_split((struct mem_block *) min, min->block_header.size, nbytes);
  // either return the split block or the whole block
  if (ret != NULL) {
    return ret;
  } else {
    set_used((struct mem_block *) min);
    remove_free(min);
    return (struct mem_block *) min;
  }
}


void*
worst_fit(uint nbytes)
{
  // find the largest fitting block
  struct free_block *free = free_list_head;
  struct free_block *max = NULL;
  while (free != NULL) {
    if (free->block_header.size >= nbytes) {
      if (max == NULL || max->block_header.size < free->block_header.size) {
        max = free;
      }
    }
    free = (struct free_block *) free->next_free;
  }
  if (max == NULL) {
    return NULL;
  }
  struct mem_block *ret = check_can_split((struct mem_block *) max, max->block_header.size, nbytes);
  // either return the split block or the whole block
  if (ret != NULL) {
    return ret;
  } else {
    set_used((struct mem_block *) max);
    remove_free(max);
    return (struct mem_block *) max;
  }
}

void*
fsm_find(int nbytes)
{
  switch (fsm) {
    case FIRST_FIT:
      return first_fit(nbytes);
    case BEST_FIT:
      return best_fit(nbytes);
    case WORST_FIT:
      return worst_fit(nbytes);
  }
  return NULL;
}

void
malloc_setfsm(int choice)
{
  switch (choice) {
    case 0:
      fsm = FIRST_FIT;
      break;
    case 1:
      fsm = BEST_FIT;
      break;
    case 2:
      fsm = WORST_FIT;
      break;
    default: // error handling without creating errors to handle
      fsm = FIRST_FIT;
      break;
  }
}

void*
malloc(uint nbytes)
{
  // check if we can reuse a block
  struct mem_block *found_block = fsm_find(nbytes);
  if (found_block != NULL) {
    return found_block + 1;
  }
  
  // get new block size
  uint64 block_size = align(nbytes + sizeof(struct mem_block), 16);
  uint64 total_size = align(block_size, PAGE_SZ);

  // create new block
  struct mem_block *block = (struct mem_block *) sbrk(total_size);
  strcpy(block->name, "");
  block->size = total_size;
  block->next_block = NULL;
  block->prev_block = NULL;

  add_block(block);
  // check if we can split the new block
  struct mem_block *split_block = check_can_split(block, total_size, nbytes);
  if (split_block != NULL) {
    // since we are returning the smaller block, we need to add the larger block to the free list
    set_free(block);
    add_free(block);
    return split_block + 1;
  }

  return block + 1;
}

void
free(void *ap)
{
  struct mem_block *block = ((struct mem_block *) ap) - 1;
  set_free(block);
  add_free(block);
}

void
malloc_print()
{
  struct mem_block *block = block_list_head;
  printf("-- Current Memory State --\n");
  if (block != NULL) {
    printf("[REGION %p]\n", block);
  }
  while (block != NULL) {
    printf("  [BLOCK %p-%p] %d\t%s\t'%s'\n", 
            block, 
            ((char *) block) + block->size,
            real_size(block->size),
            is_free(block) ? "[FREE]" : "[USED]",
            block->name);
    block = block->next_block;
  }

  printf("-- Free List --\n");
  struct free_block *free = free_list_head;
  while (free != NULL) {
    printf("[%p] -> ", free);
    free = (struct free_block *) free->next_free;
  }
  printf("NULL\n");
}

void
malloc_name(char *ap, char *name)
{
  char valid_name[8] = {0};
  for (int i = 0; i < 7; i++) {
    if (name[i] == '\0') {
      break;
    }
    valid_name[i] = name[i];
  }
  valid_name[7] = '\0';
  struct mem_block *block = ((struct mem_block *) ap) - 1;
  strcpy(block->name, valid_name);
}
