#include <linux/linkage.h>
#include <linux/sched.h>

asmlinkage long sys_print_tasks_mark_cameron(){

        struct task_struct *task;

        for_each_process(task){

                printk("FOR_EACH -> Name: %s, PID: [%d], State: [%li], PPID: [%d] \n", task->comm, task->pid, task->state, task->parent->pid);

                struct task_struct * t = task;

                do{
                        printk("WHILE_EACH -> Name: %s, PID: [%d], State: [%li], PPID: [%d] \n", t->comm, t->pid, t->state, t->parent->pid);

                }while_each_thread(task, t);

        }

        return 0;
}
