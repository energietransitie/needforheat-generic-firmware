#ifndef SCHEDULED_TASKS_H
#define SCHEDULED_TASKS_H

// test tasks
void taskA(void *);
void taskB(void *);
void taskC(void *);

// real tasks
void heartbeatv2_task(void *arg);
void upload_task(void *arg);
void twomes_scd41_task(void *arg);

#endif