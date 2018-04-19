//
//  tasks.h
//  yxsh
//
//  Created by Kirill on 19.04.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#ifndef _TASKS_H
#define _TASKS_H

#include <sys/types.h>
#include <stdbool.h>

#define MAXTSKS 64

typedef struct task {
  pid_t pid;
} task_t;

typedef struct tasks_env {
  task_t* tasks[MAXTSKS];
  size_t tasks_size;
} tasks_env_t;

/**
 * Creates new tasks environment.
 *
 * @param env Result of creation.
 */
void tasks_create_env(tasks_env_t* env);

/**
 * Creates new task and stores it to environment.
 *
 * @param pid Process ID of task.
 * @param env Current environment.
 *
 * @return true if task created.
 */
bool tasks_create_task(pid_t pid, tasks_env_t* env);

/**
 * Collects all finished (zombies) processes.
 *
 * @param env Current environment.
 */
void tasks_collect_zombies(tasks_env_t* env);

#endif /* _TASKS_H */

