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

#include "shell.h"

#define MAXTSKS 64

typedef struct task {
  pid_t pid;
  int status;
  size_t id;
  command_t* cmd;
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
 * @param cmd Executing command.
 * @param env Current environment.
 *
 * @return true if task created.
 */
bool tasks_create_task(pid_t pid, command_t* cmd, tasks_env_t* env);

/**
 * Finishes all background tasks and free memory.
 *
 * @param env Current envitonment.
 */
void tasks_release_env(tasks_env_t* env);

/**
 * Collects all finished (zombies) processes.
 *
 * @param env Current environment.
 */
void tasks_collect_zombies(tasks_env_t* env);

/**
 * Prints tasks list with ids.
 *
 * @param env Current environment.
 */
void tasks_dump_list(tasks_env_t* env);

/**
 * Searches task by specified id.
 *
 * @param id Task id.
 * @param env Encironment.
 *
 * @return Task by id or NULL, if not found.
 */
task_t* task_by_id(size_t id, tasks_env_t* env);

/**
 * Send STOP signal to background process.
 *
 * @param task Task.
 *
 * @return true if success.
 */
void task_stop(task_t* task);

/**
 * Resumes suspended task into background.
 *
 * @param task Task.
 */
void task_resume(task_t* task);

/**
 * Waits for task.
 *
 * @param task Task.
 */
void task_wait(task_t* task);

#endif /* _TASKS_H */

