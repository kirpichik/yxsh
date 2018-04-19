//
//  tasks.c
//  yxsh
//
//  Created by Kirill on 19.04.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tasks.h"

static void remove_task_by_index(size_t, tasks_env_t*);

void tasks_create_env(tasks_env_t* env) {
  memset(env, 0, sizeof(tasks_env_t));
}

bool tasks_create_task(pid_t pid, tasks_env_t* env) {
  size_t index = 0;

  if (env->tasks_size >= MAXTSKS) { // No free space to create new task
    // Try to collect zombies
    tasks_collect_zombies(env);
    // Check free space again
    if (env->tasks_size >= MAXTSKS)
      return false;
  }

  // Search for free index to insert new task
  while (index < MAXTSKS && env->tasks[index++]);

  // Free index not found
  if (index >= MAXTSKS)
    return false;

  if (!(env->tasks[index - 1] = (task_t*) malloc(sizeof(task_t))))
    return false;

  env->tasks[index - 1]->pid = pid;
  env->tasks_size++;
  return true;
}

void tasks_collect_zombies(tasks_env_t* env) {
  pid_t result;
  int status;
  size_t i = 0;
  size_t task_count = 0;

  while (task_count < env->tasks_size && i < MAXTSKS) {
    task_t* task = env->tasks[i++];
    if (!task)
      continue;

    task_count++;

    if (!(result = waitpid(task->pid, &status, WNOHANG | WUNTRACED)))
      continue;

    if (result == -1) {
      perror("yxsh: Cannot wait for process");
      continue;
    }

    if (WIFEXITED(status)) {
      remove_task_by_index(i - 1, env);
      env->tasks_size--;
      task_count--;
    }
  }
}

/**
 * Removes task from environment by index and free memory.
 *
 * @param pos Posiotion of task.
 * @param env Current environment.
 */
static void remove_task_by_index(size_t pos, tasks_env_t* env) {
  free(env->tasks[pos]);
  env->tasks[pos] = NULL;
}

