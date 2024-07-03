#pragma once

#define TASK_NUM 3

void init_tasks();
void schedule(unsigned long long sp);
extern unsigned int current_task;
