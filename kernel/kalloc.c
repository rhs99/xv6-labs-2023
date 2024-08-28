// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct {
  struct spinlock lock;
  int refcnt[PHYSTOP/PGSIZE];
} pa_refcnt;

int
pa_to_index(void *pa){
  return (uint64)pa / PGSIZE;
}

void
incr_refcnt(void *pa){
  if((char*)pa < end || (uint64)pa >= PHYSTOP || ((uint64)pa % PGSIZE) != 0 ){
    panic("Invalid physical address: incr_refcnt");
  }

  acquire(&pa_refcnt.lock);
  pa_refcnt.refcnt[pa_to_index(pa)]++;
  release(&pa_refcnt.lock);
}

void
init_refcnt(void *pa){
  if((char*)pa < end || (uint64)pa >= PHYSTOP || ((uint64)pa % PGSIZE) != 0 ){
    panic("Invalid physical address: init_refcnt");
  }

  acquire(&pa_refcnt.lock);
  pa_refcnt.refcnt[pa_to_index(pa)] = 1;
  release(&pa_refcnt.lock);
}

void
decr_refcnt(void *pa){
  if((char*)pa < end || (uint64)pa >= PHYSTOP || ((uint64)pa % PGSIZE) != 0 ){
    panic("Invalid physical address: decr_refcnt");
  }

  acquire(&pa_refcnt.lock);
  pa_refcnt.refcnt[pa_to_index(pa)]--;
  release(&pa_refcnt.lock);
}

int
get_refcnt(void *pa){
  if((char*)pa < end || (uint64)pa >= PHYSTOP || ((uint64)pa % PGSIZE) != 0 ){
    panic("Invalid physical address: get_refcnt");
  }

  acquire(&pa_refcnt.lock);
  int ret = pa_refcnt.refcnt[pa_to_index(pa)];
  release(&pa_refcnt.lock);
  return ret;
}


void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&pa_refcnt.lock, "refcnt");
  memset(pa_refcnt.refcnt, 0, sizeof(pa_refcnt.refcnt));
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  if(get_refcnt(pa) > 1){
    decr_refcnt(pa);
    return;
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    init_refcnt((void*)r);
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
