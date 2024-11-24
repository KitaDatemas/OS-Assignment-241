#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
    /* TODO: put a new process to queue [q] */
    #ifdef MLQ_SCHED
        if (q == NULL ||
            proc->prio < 0 ||
            proc->prio > 139)                           return;//Check for NULL Multilevel queue and the priority of process is invalid

        if (q[proc->prio].size >= MAX_QUEUE_SIZE)       return;//Check if priority queue is full

        int idx = q[proc->prio].size++;
        for (;
             idx - 1 >= 0 && q[proc->prio].proc[idx - 1]->priority > proc->priority;
             idx--)
            q[proc->prio].proc[idx] = q[proc->prio].proc[idx - 1] ;//Add a new process at the end of the queue has the same priority value, then increase the size
        q[proc->prio].proc[idx] = proc;
    #else
        if (q == NULL ||
                q[0].size >= MAX_QUEUE_SIZE)        return;
        int idx = q[0].size++;
        for (; idx - 1 >= 0 && q[0].proc[idx - 1]->priority > proc->priority; idx--) {
            q[0].proc[idx] = q[0].proc[idx - 1];
        }
        q[0].proc[idx] = proc;
    #endif
}

struct pcb_t * dequeue(struct queue_t * q) {
    /* TODO: return a pcb whose prioprity is the highest
     *  in the queue [q] and remember to remove it from q
     * */
    if (q == NULL)          return NULL;
    struct pcb_t * proc = NULL;

    #ifdef MLQ_SCHED
        for (int level = 0; level < MAX_PRIO; level++) {
            if (q[level].size == 0)         continue;//If queue at that level is empty, then move to next level immediately
            //Else take the first process and move the rest processes of queue to the front by one index
            proc = q[level].proc[0];

            for (int idx = 1; idx < q[level].size; idx++) {
                q[level].proc[idx - 1] = q[level].proc[idx];
                if (idx == q[level].size - 1)
                    q[level].proc[idx] = NULL;
            }

            q[level].size--;
            break;
        }
    #else
        if (q[0].size == 0)  return NULL;
        proc = q[0].proc[0];
        q[0].proc[0] = NULL;

        for (int idx = 1; idx < q[0].size; idx++)
            q[0].proc[idx - 1] = q[0].proc[idx];

        q[0].size--;
    #endif
	return proc;
}