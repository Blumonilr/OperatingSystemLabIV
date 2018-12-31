
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS+NR_PROCS; p++) {
			if(p -> isBlocked == 1) {
				continue;
			}
			if(p -> sleep_ticks > 0){
				p -> sleep_ticks--;
				continue;
			}
			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for(p=proc_table;p<proc_table+NR_TASKS+NR_PROCS;p++) {
				if(p -> isBlocked == 1) {
					continue;
				}
				//if(p -> sleep_ticks > 0){
				//	continue;
				//}
				p->ticks = p->priority;
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC int sys_process_sleep(int time, int unuse, PROCESS* p_proc)
{
        p_proc -> sleep_ticks = time * HZ /1000;
        schedule();
}

PUBLIC int sys_sem_p(SEMAPHORE* p, int unuse, PROCESS* p_proc)
{   
        p -> value--;
        if(p -> value < 0) {
          p_proc -> isBlocked = 1;
          p -> queue[p -> head] = p_proc -> pid;
          p -> head = (p -> head + 1) % 10;
          schedule();
        }
}

PUBLIC int sys_sem_v(SEMAPHORE* v, int unuse, PROCESS* p_proc)
{
        v -> value++;
        if(v -> value <= 0) {
          int index = v -> queue[v -> end];
          proc_table[index].isBlocked = 0;
          v -> end = (v -> end + 1) % 10;
          schedule();
        }
}
