//#ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct rg_elmt)
{

  struct vm_rg_struct *rg_node;
  if (rg_elmt.vmaid == 0)
    rg_node = mm->mmap->vm_freerg_list;
  else
    rg_node = mm->mmap->vm_next->vm_freerg_list;

  if ((rg_elmt.rg_start >= rg_elmt.rg_end && rg_elmt.vmaid == 0)
      && (rg_elmt.rg_end >= rg_elmt.rg_start && rg_elmt.vmaid == 1))
    return -1;

  struct vm_rg_struct * new_rg = malloc(sizeof(struct vm_rg_struct));
  new_rg->vmaid = rg_elmt.vmaid;
  new_rg->rg_start = rg_elmt.rg_start;
  new_rg->rg_end = rg_elmt.rg_end;

  if (rg_node != NULL)
    new_rg->rg_next = rg_node;

  /* Enlist the new region */
  if (rg_elmt.vmaid == 0)
    mm->mmap->vm_freerg_list = new_rg;
  else
    mm->mmap->vm_next->vm_freerg_list = new_rg;

  return 0;
}

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  // printf("Get vma by num: %d\n", vmaid);
  struct vm_area_struct *pvma= mm->mmap;

  if(mm->mmap == NULL)
    return NULL;
  int vmait = 0;
  
  while (vmait < vmaid)
  {
    if(pvma == NULL)
	    return NULL;

    vmait++;
    pvma = pvma->vm_next;
  }

  return pvma;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  // printf("Get region vmaid %d: rg start: %ld, rg end: %ld\n", mm->symrgtbl[rgid].vmaid, mm->symrgtbl[rgid].rg_start, mm->symrgtbl[rgid].rg_end);
  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;
  // printf("rgid inside __alloc: %d\n", rgid);
  /* TODO: commit the vmaid */
  rgnode.vmaid = vmaid;

  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;

    caller->mm->symrgtbl[rgid].vmaid = rgnode.vmaid;

    *alloc_addr = rgnode.rg_start;

    #ifdef PAGETBL_DUMP
     print_pgtbl(caller, 0, -1); //print max TBL
    #endif
    return 0;
  }
  /* TODO: get_free_vmrg_area FAILED handle the region management (Fig.6)*/
  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  // int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  int inc_limit_ret;

  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
//  int old_sbrk = cur_vma->sbrk;

  /* TODO INCREASE THE LIMIT
   * inc_vma_limit(caller, vmaid, inc_sz)
   */
  if (inc_vma_limit(caller, vmaid, size, &inc_limit_ret) == -1) {
    return -1;
  }
  printf("End in alloc: %ld\n", get_vma_by_num(caller->mm, vmaid)->vm_end);
  // if (vmaid == 0)
  //   printf("Put reg vmaid: %d, start: %ld, end: %ld to free rg list\n", caller->mm->mmap->vm_freerg_list->vmaid, caller->mm->mmap->vm_freerg_list->rg_start, caller->mm->mmap->vm_freerg_list->rg_end);
  // else
  //   printf("Put reg vmaid: %d, start: %ld, end: %ld to free rg list\n", caller->mm->mmap->vm_next->vm_freerg_list->vmaid, caller->mm->mmap->vm_next->vm_freerg_list->rg_start, caller->mm->mmap->vm_next->vm_freerg_list->rg_end);
  

  /* TODO: commit the limit increment */
  get_free_vmrg_area(caller, vmaid, size, &rgnode);
  // printf("vmaid: %d, rgnode vmaid: %d\n", vmaid, rgnode.vmaid);
  caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
  caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
  caller->mm->symrgtbl[rgid].vmaid = rgnode.vmaid;
  printf("Get region in alloc rgid %d vmaid %d: rg start: %ld, rg end: %ld\n", rgid, caller->mm->symrgtbl[rgid].vmaid, caller->mm->symrgtbl[rgid].rg_start, caller->mm->symrgtbl[rgid].rg_end);

  #ifdef PAGETBL_DUMP
     print_pgtbl(caller, 0, -1); //print max TBL
  #endif
  /* TODO: commit the allocation address 
  // *alloc_addr = ...
  */
  *alloc_addr = rgnode.rg_start;
  return 0;
}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __free(struct pcb_t *caller, int rgid)
{
  struct vm_rg_struct rgnode;
  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;

  /* TODO: Manage the collect freed region to freerg_list */
  if (get_symrg_byid(caller->mm, rgid) == NULL) return -1;
  rgnode = *get_symrg_byid(caller->mm, rgid);

  /*enlist the obsoleted memory region */
  printf("Put free rg vmaid %d: rg start: %ld, rg end: %ld\n", rgnode.vmaid, rgnode.rg_start, rgnode.rg_end);
  enlist_vm_freerg_list(caller->mm, rgnode);

  return 0;
}

/*pgalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int pgalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  /* By default using vmaid = 0 */
  return __alloc(proc, 0, reg_index, size, &addr);
}

/*pgmalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify vaiable in symbole table)
 */
int pgmalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  /* By default using vmaid = 1 */
  return __alloc(proc, 1, reg_index, size, &addr);
}

/*pgfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int pgfree_data(struct pcb_t *proc, uint32_t reg_index)
{
   return __free(proc, reg_index);
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];
 
  if (!PAGING_PTE_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    int vicpgn, swpfpn; 
    int vicfpn;
    uint32_t vicpte;

    int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    find_victim_page(caller->mm, &vicpgn);
    if (vicpgn == -1)
        return -1;
    vicpte = mm->pgd[vicpgn];
    vicfpn = PAGING_PTE_PGN(vicpte);

    /* Get free frame in MEMSWP */
    MEMPHY_get_freefp(caller->active_mswp, &swpfpn);
    if (swpfpn == -1)
        return -1;


    /* Do swap frame from MEMRAM to MEMSWP and vice versa*/
    /* Copy victim frame to swap */
    __swap_cp_page(caller->mram, vicfpn, *(caller->mswp), swpfpn);
    /* Copy target frame from swap to mem */
    __swap_cp_page(*(caller->mswp), tgtfpn, caller->mram, vicfpn);

    /* Update page table */
    pte_set_swap(&mm->pgd[vicpgn], 1, PAGING_OFFST(vicpgn));

    /* Update its online status of the target page */
//    pte_set_fpn() & mm->pgd[pgn];
    pte_set_fpn(&mm->pgd[pgn], tgtfpn);

    enlist_pgn_node(&caller->mm->fifo_pgn,pgn);
  }

  *fpn = PAGING_PTE_FPN(pte);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess 
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if(pg_getpage(mm, pgn, &fpn, caller) != 0) 
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_read(caller->mram,phyaddr, data);

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess 
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if(pg_getpage(mm, pgn, &fpn, caller) != 0) 
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_write(caller->mram,phyaddr, value);

   return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region 
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __read(struct pcb_t *caller, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  int vmaid = currg->vmaid;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}


/*pgwrite - PAGING-based read a region memory */
int pgread(
		struct pcb_t * proc, // Process executing the instruction
		uint32_t source, // Index of source register
		uint32_t offset, // Source address = [source] + [offset]
		uint32_t destination) 
{
  BYTE data;
  int val = __read(proc, source, offset, &data);

  destination = (uint32_t) data;
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region 
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __write(struct pcb_t *caller, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  int vmaid = currg->vmaid;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  
  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*pgwrite - PAGING-based write a region memory */
int pgwrite(
		struct pcb_t * proc, // Process executing the instruction
		BYTE data, // Data to be wrttien into memory
		uint32_t destination, // Index of destination register
		uint32_t offset)
{
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return __write(proc, destination, offset, data);
}


/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PTE_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct* get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
  struct vm_rg_struct * newrg;

  /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);//Lay ra vma hien tai

  if (cur_vma == NULL)
      return NULL;

  newrg = malloc(sizeof(struct vm_rg_struct));

  /* TODO: update the newrg boundary
  */
  newrg->vmaid = vmaid;

  if (vmaid) {
      newrg->rg_start = cur_vma->sbrk;
      newrg->rg_end = cur_vma->sbrk - size;

      cur_vma->sbrk -= size;
  }
  else {
    // printf("cur_vma->sbrk: %ld, cur_vma->end: %ld\n", cur_vma->sbrk, cur_vma->vm_end);

      newrg->rg_start = cur_vma->sbrk;
      newrg->rg_end = cur_vma->sbrk + size;

      // printf("area->rg start: %ld, ", newrg->rg_start);
      // printf("area->rg end: %ld\n", newrg->rg_end);

      cur_vma->sbrk += size;
  }
  newrg->rg_next = NULL;
  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
  struct vm_area_struct *vma = caller->mm->mmap;

  /* TODO validate the planned memory area is not overlapped */
  while (vma) {
    if (OVERLAP(vmastart, vmaend, vma->vm_start, vma->vm_end)/*&& (vma->vm_start!=vma->vm_end)*/){
      printf("Vma start: %ld, Vma end: %ld is overlapped with vma start: %ld, vma end: %ld\n", vmastart, vmaend, vma->vm_start, vma->vm_end);
      return -1;
    }
    vma = vma->vm_next;
  }

  return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size 
 *@inc_limit_ret: increment limit return
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz, int* inc_limit_ret)
{
  struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
  int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
  int incnumpage =  inc_amt / PAGING_PAGESZ;
  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);// Xin them rg node moi tai sbrk cua vma
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  int old_end = cur_vma->vm_end;
  /*Validate overlap of obtained region */
  int new_start_vma = area->rg_start;

  if (vmaid == 1) {
    if (area->rg_start > old_end)
      new_start_vma = old_end;
  } else if (vmaid == 0) {
    if (area->rg_start < old_end)
      new_start_vma = old_end;
  }

  if (validate_overlap_vm_area(caller, vmaid, new_start_vma, area->rg_end) < 0){//Kiem tra viec xin them co bi overlap voi vung khac hay khong
    return -1; /*Overlap and failed allocation */
  }
  /* TODO: Obtain the new vm area based on vmaid */
  //cur_vma->vm_end... 
  // inc_limit_ret...
  printf("incsz: %d, old_end: %ld, old sbrk: %ld\n", inc_sz, old_end, area->rg_start);
  inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz - (old_end - area->rg_start));
  incnumpage =  inc_amt / PAGING_PAGESZ;
#ifdef MM_PAGING_HEAP_GODOWN
  printf("End in inc_vma: %ld\n", get_vma_by_num(caller->mm, vmaid)->vm_end);
  if (vmaid == 1) {
    if (area->rg_end < cur_vma->vm_end)//Tang kich thuoc vma neu viec cap phat reg hop le
      cur_vma->vm_end -= inc_amt;
  }
  else if (vmaid == 0) {
    if (area->rg_end > cur_vma->vm_end)
      cur_vma->vm_end += inc_amt;
  }
  printf("End in inc_vma: %ld\n", get_vma_by_num(caller->mm, vmaid)->vm_end);
#else
  if (vmaid == 1) {
    if (area->rg_end > cur_vma->vm_end)
      cur_vma->vm_end += inc_amt;
  }
  else if (vmaid == 0) {
    if (area->rg_end > cur_vma->vm_end)
      cur_vma->vm_end += inc_amt;
  }
#endif

  *inc_limit_ret = cur_vma->vm_end;

  
  printf("Increase page num: %d %d\n", incnumpage, inc_amt);

  printf("Old end in alloc before: %ld\n", old_end);
  if (vm_map_ram(caller, area->rg_start, area->rg_end, 
                    old_end, incnumpage , newrg, vmaid) < 0) {
    free(area);
    return -1; /* Map the memory to MEMRAM */
  }
  printf("Old end in alloc after: %ld\n", old_end);
  enlist_vm_freerg_list(caller->mm, *area);  
  free(area);
  free(newrg);

  if (vmaid == 0)
    printf("Put reg inside inc vma vmaid: %d, start: %ld, end: %ld to free rg list\n", caller->mm->mmap->vm_freerg_list->vmaid, caller->mm->mmap->vm_freerg_list->rg_start, caller->mm->mmap->vm_freerg_list->rg_end);
  else
    printf("Put reg inside inc vma vmaid: %d, start: %ld, end: %ld to free rg list\n", caller->mm->mmap->vm_next->vm_freerg_list->vmaid, caller->mm->mmap->vm_next->vm_freerg_list->rg_start, caller->mm->mmap->vm_next->vm_freerg_list->rg_end);
  
  return 0;

}

/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn) 
{
  struct pgn_t **pg = &(mm->fifo_pgn),
                *deletePage;

  /* TODO: Implement the theorical mechanism to find the victim page */
  if (*pg == NULL)      return -1;
  while ((*pg)->pg_next != NULL) // Vì fifo_pgn là danh sách các trang đã được sử dụng, khi thêm 1 trang sử dụng mới sẽ được thêm vào đầu. Do đó cần chọn thằng lâu nhất được add vào
      pg = &((*pg)->pg_next);
  *retpgn = (*pg)->pgn;
  deletePage = (*pg);
  *pg = NULL;

  free(deletePage);

  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size 
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
    return -1;

  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;
  printf ("Find free region\n");
  if (rgit != NULL)
    // printf ("rg vmaid: %d, vmaid: %d, rg in free rg list start: %ld, end: %ld\n", rgit->vmaid, vmaid, rgit->rg_start, rgit->rg_end);
  /* Traverse on list of free vm region to find a fit space */
  while (rgit != NULL && rgit->vmaid == vmaid)
  {
    // printf("Traverse at rg has vmaid %d we want to find vmaid %d\n", rgit->vmaid, vmaid);
      if (rgit->vmaid == 0) {
        // printf("rg start: %ld, rg end: %ld, size: %d\n", rgit->rg_start, rgit->rg_end, size);
        if (rgit->rg_start + size <= rgit->rg_end)
        { /* Current region has enough space */
          newrg->rg_start = rgit->rg_start;
          newrg->rg_end = rgit->rg_start + size;

          /* Update left space in chosen region */
          if (rgit->rg_start + size < rgit->rg_end)
          {
            rgit->rg_start = rgit->rg_start + size;
          }
          else
          { /*Use up all space, remove current node */
            /*Clone next rg node */
            struct vm_rg_struct *nextrg = rgit->rg_next;

            /*Cloning */
            if (nextrg != NULL)
            {
              rgit->rg_start = nextrg->rg_start;
              rgit->rg_end = nextrg->rg_end;

              rgit->rg_next = nextrg->rg_next;

              free(nextrg);
            }
            else
            { /*End of free list */
              rgit->rg_start = rgit->rg_end;	//dummy, size 0 region
              rgit->rg_next = NULL;
            }
          }
          break;
        }
        else
        {
          rgit = rgit->rg_next;	// Traverse next rg
        }
      } else if (rgit->vmaid == 1) {
        // printf("rg start: %ld, rg end: %ld, size: %d\n", rgit->rg_start, rgit->rg_end, size);
        if (rgit->rg_start - size >= rgit->rg_end)
        { /* Current region has enough space */
          newrg->rg_start = rgit->rg_start;
          newrg->rg_end = rgit->rg_start - size;

          /* Update left space in chosen region */
          if (rgit->rg_start - size > rgit->rg_end)
          {
            rgit->rg_start = rgit->rg_start - size;
          }
          else
          { /*Use up all space, remove current node */
            /*Clone next rg node */
            struct vm_rg_struct *nextrg = rgit->rg_next;

            /*Cloning */
            if (nextrg != NULL)
            {
              rgit->rg_start = nextrg->rg_start;
              rgit->rg_end = nextrg->rg_end;

              rgit->rg_next = nextrg->rg_next;

              free(nextrg);
            }
            else
            { /*End of free list */
              rgit->rg_start = rgit->rg_end;	//dummy, size 0 region
              rgit->rg_next = NULL;
            }
          }
          break;
        }
        else
        {
          rgit = rgit->rg_next;	// Traverse next rg
        }
      }
    }

 if(newrg->rg_start == -1) // new region not found
   return -1;
  printf("Found suitable reg\n");
 return 0;
}

//#endif
