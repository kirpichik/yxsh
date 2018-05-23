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
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <readline/readline.h>

#include "tasks.h"

#define STATUS_RUNNING 0
#define STATUS_STOPPED 1
#define STATUS_SIGNAL  2
#define STATUS_DONE    3
#define STATUS_CRASHED 4

static bool update_task_status(task_t*);
static void print_task(task_t*);
static char* get_status_description(int);
static int translate_status(int);
static void remove_task_by_index(size_t, tasks_env_t*);

void tasks_create_env(tasks_env_t* env) {
  memset(env, 0, sizeof(tasks_env_t));
}

bool tasks_run_task(tasks_env_t* env, pid_t pgid, bool bg, char* display) {
  pid_t* pids = (pid_t*) malloc(sizeof(pid_t));
  pids[0] = pgid;
  return tasks_run_pipeline(env, pgid, pids, bg, 1, display);
}

bool tasks_run_pipeline(tasks_env_t* env, pid_t pgid, pid_t* pids,
    bool bg, size_t num, char* display) {
  size_t index = 0;
  bool finished;
  task_t* task;

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

  task = env->tasks[index - 1];

  task->display_name = display;
  task->pgid = pgid;
  task->id = index;
  task->status = STATUS_RUNNING;
  task->count = num;
  task->pids = pids;
  task->pids_amount = num;

  env->tasks_size++;

  finished = bg ? !update_task_status(task) : task_wait(task);
  if (bg || !finished) {
    if (!bg)
      rl_crlf();
    print_task(task);
  }

  if (finished)
    remove_task_by_index(index - 1, env);

  return true;
}

bool tasks_has_free(tasks_env_t* env) {
  if (env->tasks_size >= MAXTSKS)
    tasks_collect_zombies(env);
  return env->tasks_size < MAXTSKS;
}

void tasks_release_env(tasks_env_t* env) {
  size_t i = 0;
  size_t task_count = 0;
  int status;
  signal(SIGCHLD, SIG_DFL);

  while (task_count < env->tasks_size && i < MAXTSKS) {
    task_t* task = env->tasks[i++];
    if (!task)
      continue;
    task_count++;
    killpg(task->pgid, SIGHUP);

    if (waitpid(task->pgid, &status, WNOHANG | WUNTRACED) != -1)
      task->status = translate_status(status);
    remove_task_by_index(i - 1, env);
    task_count--;
  }
}

void tasks_collect_zombies(tasks_env_t* env) {
  size_t i = 0;
  size_t task_count = 0;

  while (task_count < env->tasks_size && i < MAXTSKS) {
    task_t* task = env->tasks[i++];
    if (!task)
      continue;
    task_count++;

    if ((task->status != STATUS_RUNNING &&
         task->status != STATUS_STOPPED) ||
        !update_task_status(task)) {
      print_task(task);
      remove_task_by_index(i - 1, env);
      task_count--;
    }
  }
}

bool tasks_update_status(tasks_env_t* env) {
  int status;
  pid_t pid;
  size_t i;
  size_t task_count;
  bool print = false;

  while ((pid = wait(&status)) > 0) {
    task_count = 0;
    i = 0;
    // Search for task with received pid
    while (task_count < env->tasks_size && i < MAXTSKS) {
      task_t* task = env->tasks[i++];
      if (!task)
        continue;
      task_count++;

      // Search for pid in this task
      for (size_t j = 0; j < task->pids_amount; j++) {
        if (pid == task->pids[j]) {
          if (--task->count == 0) {
            task->status = translate_status(status);
            rl_crlf();
            print_task(task);
            print = true;
            if (task->status != STATUS_STOPPED && task->status != STATUS_RUNNING)
              remove_task_by_index(i - 1, env);
          }
          break;
        }
      }
    }
  }

  return print;
}

void tasks_dump_list(tasks_env_t* env) {
  size_t i = 0;
  size_t task_count = 0;

  while (task_count < env->tasks_size && i < MAXTSKS) {
    task_t* task = env->tasks[i++];
    if (!task)
      continue;
    task_count++;
    print_task(task);
  }
}

task_t* task_by_id(tasks_env_t* env, size_t id) {
  return id - 1 < MAXTSKS ? env->tasks[id - 1] : NULL;
}

void task_resume_background(task_t* task) {
  task->status = STATUS_RUNNING;
  print_task(task);
  if (killpg(task->pgid, SIGCONT)) {
    perror("yxsh: Cannot resume task");
    return;
  }
}

void task_resume_foreground(tasks_env_t* env, task_t* task) {
  signal(SIGCHLD, SIG_DFL);
  if (!setup_terminal(task->pgid))
    return;
  if (killpg(task->pgid, SIGCONT)) {
    perror("yxsh: Cannot resume task");
    setup_terminal(getpgrp());
    return;
  }
  signal(SIGINT, SIG_DFL);
  if (task_wait(task))
    remove_task_by_index(task->id - 1, env);
  else {
    rl_crlf();
    print_task(task);
  }
  setup_terminal(getpgrp());
}

bool task_wait(task_t* task) {
  int status;
  bool repeat = true;
  while (repeat) {
    pid_t result = waitpid(-task->pgid, &status, WUNTRACED);
    if (result != -1) {
        if (WIFSTOPPED(status)) {
          task->status = STATUS_STOPPED;
          return false;
        } else {
          task->status = translate_status(status);
          repeat = --task->count != 0;
        }
    } else {
      perror("yxsh: Cannot wait for process");
      return true;
    }
  }

  return true;
}

bool setup_terminal(pid_t pid) {
  signal(SIGTTOU, SIG_IGN);
  if (tcsetpgrp(STDIN_FILENO, pid)) {
    perror("yxsh: Cannot setup terminal foreground");
    signal(SIGTTOU, SIG_DFL);
    return false;
  }

  signal(SIGTTOU, SIG_DFL);
  return true;
}

/**
 * Updates task status.
 *
 * @param task Task.
 *
 * @return true if task is running or stopped and pipeline is not finished.
 */
static bool update_task_status(task_t* task) {
  pid_t result;
  int status;
  if (!(result = waitpid(-task->pgid, &status, WNOHANG | WUNTRACED)))
    return true;

  if (result == -1) {
    perror("yxsh: Cannot wait for process");
    return false;
  }

  task->status = translate_status(status);
  return task->status == STATUS_STOPPED
      || task->status == STATUS_RUNNING
      || --task->count != 0;
}

/**
 * Prints task to stderr.
 *
 * @param task Task.
 */
static void print_task(task_t* task) {
  char* status = get_status_description(task->status);
  fprintf(stderr, "[%lu] (%d | %s): %s\n", (unsigned long) task->id,
      (int) task->pgid, status, task->display_name);
}

/**
 * Translates status from system status returned from waitpid to
 * local status defines.
 *
 * @param status Input status.
 *
 * @return Result status.
 */
static int translate_status(int status) {
  if (WIFEXITED(status)) {
    if (WEXITSTATUS(status) != 0)
      return STATUS_CRASHED;
    return STATUS_DONE;
  }

  if (WIFSIGNALED(status))
    return STATUS_SIGNAL;

  if (WIFSTOPPED(status))
    return STATUS_STOPPED;

  return STATUS_RUNNING;
}

/**
 * Looks for a suitable description string for process status.
 *
 * @param status Status.
 *
 * @return Description string.
 */
static char* get_status_description(int status) {
  switch (status) {
    case STATUS_CRASHED:
      return "Crashed";
    case STATUS_DONE:
      return "Done";
    case STATUS_SIGNAL:
      return "Terminated";
    case STATUS_STOPPED:
      return "Stopped";
  }
  return "Running";
}

/**
 * Removes task from environment by index and free memory.
 *
 * @param pos Position of the task.
 * @param env Current environment.
 */
static void remove_task_by_index(size_t pos, tasks_env_t* env) {
  free(env->tasks[pos]->display_name);
  free(env->tasks[pos]->pids);
  free(env->tasks[pos]);
  env->tasks_size--;
  env->tasks[pos] = NULL;
}

