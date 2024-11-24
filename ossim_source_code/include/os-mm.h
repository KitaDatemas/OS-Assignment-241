#ifndef OSMM_H
#define OSMM_H

#define MM_PAGING
#define PAGING_MAX_MMSWP 4 /* max number of supported swapped space */
#define PAGING_MAX_SYMTBL_SZ 30

typedef char BYTE;
typedef uint32_t addr_t;
//typedef unsigned int uint32_t;

struct pgn_t{
   int pgn;
   struct pgn_t *pg_next; 
};

/*
 *  Memory region struct
 */
struct vm_rg_struct {
   int vmaid;

   unsigned long rg_start;
   unsigned long rg_end;

   struct vm_rg_struct *rg_next;
};

/*
 *  Memory area struct
 */
struct vm_area_struct {
   unsigned long vm_id;
   unsigned long vm_start;
   unsigned long vm_end;

   unsigned long sbrk;
/*
 * Derived field
 * unsigned long vm_limit = vm_end - vm_start
 */
   struct mm_struct *vm_mm;//truy xuất ngược lên mm_struct quản lý vùng nhớ ảo đó.
   struct vm_rg_struct *vm_freerg_list;//Lưu trữ các vùng nhớ ảo còn trống
   struct vm_area_struct *vm_next;//Đường dẫn tới vùng nhớ ảo tiếp theo
};

/* 
 * Memory management struct
 */
struct mm_struct {
   uint32_t *pgd;//Chứa địa chỉ của page table
   //PAGE ENTRY | FRAME TRONG PHYSICAL MEMORY

   struct vm_area_struct *mmap;//Linked list dẫn tới head của vùng nhớ ảo (virtual memory area)

   /* Currently we support a fixed number of symbol */
   struct vm_rg_struct symrgtbl[PAGING_MAX_SYMTBL_SZ];//Bảng với số phần tử cố định các vùng nhớ ảo (used virtual memory region) đã sử dụng

   /* list of free page */
   struct pgn_t *fifo_pgn;//TODO TÌM HIỂU SAU NÀY THẦY GỢI Ý
};

/*
 * FRAME/MEM PHY struct
 */
struct framephy_struct { 
   int fpn;//ID của frame: Cho biết offset của frame trong physical memory
   struct framephy_struct *fp_next;//Con trỏ tới địa chỉ của frame tiếp theo

   /* Reserved for tracking allocated framed */
   struct mm_struct* owner;//Trong trường hợp frame đang được sử dụng, owner để truy xuất tới mm_struct đang quản lý frame này
};

struct memphy_struct {
   /* Basic field of data and size */
   BYTE *storage;
   int maxsz;
   
   /* Sequential device fields */ 
   int rdmflg;
   int cursor;

   /* Management structure */
   struct framephy_struct *free_fp_list;
   struct framephy_struct *used_fp_list;
};

#endif
