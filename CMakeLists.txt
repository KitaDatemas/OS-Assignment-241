cmake_minimum_required(VERSION 3.27)
project(BTL C)

set(CMAKE_C_STANDARD 11)

include_directories(ossim_source_code/include)

add_executable(BTL
        ossim_source_code/include/bitops.h
        ossim_source_code/include/common.h
        ossim_source_code/include/cpu.h
        ossim_source_code/include/loader.h
        ossim_source_code/include/mem.h
        ossim_source_code/include/mm.h
        ossim_source_code/include/os-cfg.h
        ossim_source_code/include/os-mm.h
        ossim_source_code/include/queue.h
        ossim_source_code/include/sched.h
        ossim_source_code/include/timer.h
        ossim_source_code/src/cpu.c
        ossim_source_code/src/loader.c
        ossim_source_code/src/mem.c
        ossim_source_code/src/mm-memphy.c
        ossim_source_code/src/mm-vm.c
        ossim_source_code/src/mm.c
        ossim_source_code/src/os.c
        ossim_source_code/src/paging.c
        ossim_source_code/src/queue.c
        ossim_source_code/src/sched.c
        ossim_source_code/src/timer.c)
