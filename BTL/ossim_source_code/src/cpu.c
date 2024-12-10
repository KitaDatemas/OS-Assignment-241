#include "stdio.h"
#include "cpu.h"
#include "mem.h"
#include "mm.h"

int calc(struct pcb_t *proc)
{
  return ((unsigned long)proc & 0UL);
}

int alloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  addr_t addr = alloc_mem(size, proc);
  if (addr == 0)
  {
    return 1;
  }
  else
  {
    proc->regs[reg_index] = addr;
    return 0;
  }
}

int free_data(struct pcb_t *proc, uint32_t reg_index)
{
  return free_mem(proc->regs[reg_index], proc);
}

int read(
    struct pcb_t *proc, /* Process executing the instruction */
    uint32_t source,    /* Index of source register */
    uint32_t offset,    /* Source address = [source] + [offset] */
    uint32_t destination)
{ /* Index of destination register */
  BYTE data;
  if (read_mem(proc->regs[source] + offset, proc, &data))
  {
    proc->regs[destination] = data;
    return 0;
  }
  else
  {
    return 1;
  }
}

int write(
    struct pcb_t *proc,   /* Process executing the instruction */
    BYTE data,            /* Data to be wrttien into memory */
    uint32_t destination, /* Index of destination register */
    uint32_t offset)
{ /* Destination address = [destination] + [offset] */
  return write_mem(proc->regs[destination] + offset, proc, data);
}

int run(struct pcb_t *proc)
{
  /* Check if Program Counter point to the proper instruction */
  if (proc->pc >= proc->code->size)
  {
    return 1;
  }

  struct inst_t ins = proc->code->text[proc->pc];
  proc->pc++;
  int stat = 1;
  switch (ins.opcode)
  {
  case CALC:
    printf("---------------CALC------------------\n");
    stat = calc(proc);
    break;
  case ALLOC:
    printf("-----------ALLOC size %d put at reg %d-------------\n", ins.arg_0, ins.arg_1);
#ifdef MM_PAGING
    stat = pgalloc(proc, ins.arg_0, ins.arg_1);
    if (stat == -2)
      printf("Can not alloc due to reallocate to the same reg\n");

#else /* not MM_PAGING*/
    stat = alloc(proc, ins.arg_0, ins.arg_1);
    printf("Can not alloc due to reallocate to the same reg\n");
#endif /* MM_PAGING */
    printf("-----------END OF ALLOC-------------\n");
    break;
#ifdef MM_PAGING
  case MALLOC:
    printf("-----------MALLOC size %d put at reg %d-------------\n", ins.arg_0, ins.arg_1);
    stat = pgmalloc(proc, ins.arg_0, ins.arg_1);
    if (stat < 0)
      printf("Can't initialize memory\n");
    printf("-----------END OF MALLOC-------------\n");
    break;
#endif /* MM_PAGING */
  case FREE:
    printf("-----------FREE at reg %d-------------\n", ins.arg_0);
#ifdef MM_PAGING
    stat = pgfree_data(proc, ins.arg_0);
    if (stat != 0)
      printf("Free an invalid region\n");
#else /* not MM_PAGING*/
    stat = free_data(proc, ins.arg_0);
    if (stat != 0)
      printf("Free an invalid region\n");
#endif /* MM_PAGING */
    printf("------------END OF FREE--------------\n");
    break;
  case READ:
#ifdef MM_PAGING
    printf("-----------READ at reg %d with offset %d put to reg %d -------------\n", ins.arg_0, ins.arg_1, ins.arg_2);
    stat = pgread(proc, ins.arg_0, ins.arg_1, ins.arg_2);
    if (stat != 0)
      printf("Invalid read region\n");
#else /* not MM_PAGING*/
    stat = read(proc, ins.arg_0, ins.arg_1, ins.arg_2);
    if (stat != 0)
      printf("Invalid read region\n");
#endif /* MM_PAGING */
    printf("------------END OF READ--------------\n");
    break;
  case WRITE:
#ifdef MM_PAGING
    printf("-----------WRITE data %d put to reg %d with offset %d-------------\n", ins.arg_0, ins.arg_1, ins.arg_2);
    stat = pgwrite(proc, ins.arg_0, ins.arg_1, ins.arg_2);
    if (stat != 0)
      printf("Invalid write region\n");
#else /* not MM_PAGING*/
    stat = write(proc, ins.arg_0, ins.arg_1, ins.arg_2);
    if (stat != 0)
      printf("Invalid write region\n");
#endif /* MM_PAGING */
    printf("-------------END OF WRITE---------------\n");
    break;
  case ADDR:
    printf("-----------ADDR at reg %d-------------\n", ins.arg_0);
#ifdef MM_PAGING
    stat = pgaddr(proc, ins.arg_0);
#else /* not MM_PAGING*/
    // stat = free_data(proc, ins.arg_0);
#endif /* MM_PAGING */
    printf("------------END OF ADDR--------------\n");
    break;
  default:
    stat = 1;
  }
  return stat;
}
