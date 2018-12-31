
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"



SEMAPHORE customers, barbers, mutex, S1, S2;


int chairs;
int customerID;
/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	chairs = 2;
	customerID=0;

	customers.value = 0;
	barbers.value = 0;
	mutex.value = 1;
	S1.value = 0;//理发师开始理发
	S2.value = 1;//理发师结束理发

	customers.head = customers.end = barbers.head = barbers.end = mutex.head = mutex.end = S1.end = S1.head = S2.head = S2.end = 0;

	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;

	int i;
        u8              privilege;
        u8              rpl;
        int             eflags;

    //printf("%d", 1111);
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
                if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
                }
                else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
                }

		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		//p_proc->nr_tty = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;

		p_proc -> sleep_ticks = 0;
		p_proc -> isBlocked = 0;
	}

	proc_table[0].ticks = proc_table[0].priority = 1500;
	proc_table[1].ticks = proc_table[1].priority =  10;
	proc_table[2].ticks = proc_table[2].priority =  10;
	proc_table[3].ticks = proc_table[3].priority =  10;
	proc_table[4].ticks = proc_table[4].priority =  10;
	proc_table[5].ticks = proc_table[5].priority =  10;

        proc_table[1].nr_tty = 0;
        proc_table[2].nr_tty = 0;
        proc_table[3].nr_tty = 0;
        proc_table[4].nr_tty = 0;
	proc_table[5].nr_tty = 0;


	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}

int customerID=0;

void barber(){
	p(&S2);
	p(&customers);	
	p(&mutex);
	chairs++;  
	v(&mutex);
 		
	printf("Barber works.\n");
	v(&barbers);
	process_sleep(100);	
	printf("Barber finishes.\n");
	v(&S1);
}

void customer(int sleep){
	p(&mutex);
	customerID++;
	int id = customerID;
	if(chairs>0) {
		chairs--;  
		v(&mutex);
		printf("Customer %d arrives and wait.\n", id);
		v(&customers);
		p(&barbers);    
		printf("Customer %d is cutting hair.\n", id);
		p(&S1);
		printf("Customer %d has haircut and leaves.\n", id);
		v(&S2);
	} else {
		v(&mutex);
		printf("Customer %d arrives but leaves without haircut.\n", id);
	}
	process_sleep(sleep);
}


/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	while (1) {
		//printf("<Ticks:%x>", get_ticks());
		//milli_delay(200);
		//process_sleep(2000);
	}
}

/*======================================================================*
                               TestB(Customer1)
 *======================================================================*/
void TestB()
{
	while(1){
		customer(200);
	}
}

/*======================================================================*
                               TestC(Customer2)
 *======================================================================*/
void TestC()
{
	while(1){
		//printf("C");
		//milli_delay(200);
		customer(100);
	}
}

/*======================================================================*
                               TestD(Customer3)
 *======================================================================*/
void TestD()
{
	while(1){
		//printf("D");
		//milli_delay(200);
		customer(150);
	}
}


/*======================================================================*
                               TestE(barber)
 *======================================================================*/
void TestE()
{
	while(1){
		//printf("E");
		//milli_delay(200);
		barber();
	}
}
