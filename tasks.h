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

#define MAXTSKS 1024

typedef struct task {
  pid_t pgid;
  int status;
  size_t id;
  size_t count;
  char* display_name;
  size_t pids_amount;
  pid_t* pids;
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
 * Runs the task and monitors its status until it finishes.
 *
 * @param env Current environment.
 * @param pgid Process group ID of task.
 * @param bg Is this task background.
 * @param display Task display name.
 *
 * @return true if task created.
 */
bool tasks_run_task(tasks_env_t* env, pid_t pgid, bool bg, char* display);

/**
 * Runs the pipeline task and monitors its status until it finishes.
 *
 * @param env Current environment.
 * @param pgid Process group ID of pipeline group.
 * @param pids Process IDs for all line.
 * @param bg Is this task background.
 * @param num Number of commands in pipeline.
 * @param display Task display name.
 *
 * @return true if task created.
 */
bool tasks_run_pipeline(tasks_env_t* env, pid_t pgid, pid_t* pids,
    bool bg, size_t num, char* display);

/**
 * Checks for avaliable task id to create new task.
 *
 * @param env Current environment.
 *
 * @return true if it has free id.
 */
bool tasks_has_free(tasks_env_t* env);

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
 * Updates process status in background tasks list.
 *
 * @param env Current environment.
 *
 * @return Is some status updates dumped.
 */
bool tasks_update_status(tasks_env_t* env);

/**
 * Prints tasks list with ids.
 *
 * @param env Current environment.
 */
void tasks_dump_list(tasks_env_t* env);

/**
 * Searches task by specified id.
 *
 * @param env Current environment.
 * @param id Task id.
 *
 * @return Task by id or NULL, if not found.
 */
task_t* task_by_id(tasks_env_t* env, size_t id);

/**
 * Resumes suspended task into background.
 *
 * @param task Task.
 */
void task_resume_background(task_t* task);

/**
 * Resumes suspended task into foreground.
 *
 * @param env Current environment.
 * @param task Task.
 */
void task_resume_foreground(tasks_env_t* env, task_t* task);

/**
 * Waits for task.
 *
 * @param task Task.
 *
 * @return true if task finished.
 */
bool task_wait(task_t* task);

/**
 * Setting up pid to stdin.
 *
 * @param pid Process id to setup.
 *
 * @return true if success.
 */
bool setup_terminal(pid_t pid);

#endif /* _TASKS_H */

