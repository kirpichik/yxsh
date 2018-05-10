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
static bool cmddup(command_t*, command_t*);

void tasks_create_env(tasks_env_t* env) {
  memset(env, 0, sizeof(tasks_env_t));
}

bool tasks_create_task(pid_t pid, command_t* cmd, tasks_env_t* env, bool bg) {
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

  task->pid = pid;
  if (!cmddup(&task->cmd, cmd))
    return false;
  task->id = index;
  env->tasks_size++;
  finished = bg ? !update_task_status(task) : task_wait(task);
  if (bg || !finished)
    print_task(task);

  if (finished)
    remove_task_by_index(index - 1, env);

  return true;
}

void tasks_release_env(tasks_env_t* env) {
  size_t i = 0;
  size_t task_count = 0;

  while (task_count < env->tasks_size && i < MAXTSKS) {
    task_t* task = env->tasks[i++];
    if (!task)
      continue;
    task_count++;
    kill(task->pid, SIGHUP);

    update_task_status(task);
    print_task(task);
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

task_t* task_by_id(size_t id, tasks_env_t* env) {
  return id <= env->tasks_size ? env->tasks[id - 1] : NULL;
}

void task_resume_background(task_t* task) {
  if (kill(task->pid, SIGCONT)) {
    perror("yxsh: Cannot resume task");
    return;
  }
  task->status = STATUS_RUNNING;
  print_task(task);
}

void task_resume_foreground(task_t* task) {
  if (!setup_terminal(task->pid))
    return;
  if (kill(task->pid, SIGCONT)) {
    setup_terminal(task->pid);
    perror("yxsh: Cannot resume task");
    return;
  }
  task_wait(task);
  setup_terminal(getpgrp());
}

bool task_wait(task_t* task) {
  pid_t result;
  int status;
  if ((result = waitpid(task->pid, &status, WUNTRACED)) != -1) {
      if (WIFSTOPPED(status)) {
        task->status = STATUS_STOPPED;
        return false;
      } else
        task->status = translate_status(status);
  } else
    perror("yxsh: Cannot wait for process");

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
 * @return true if task is running or stopped.
 */
static bool update_task_status(task_t* task) {
  pid_t result;
  int status;
  if (!(result = waitpid(task->pid, &status, WNOHANG | WUNTRACED)))
      return true;

  if (result == -1) {
    perror("yxsh: Cannot wait for process");
    return false;
  }

  task->status = translate_status(status);
  return task->status == STATUS_STOPPED || task->status == STATUS_RUNNING;
}

/**
 * Prints task to stderr.
 *
 * @param task Task.
 */
static void print_task(task_t* task) {
  char* status = get_status_description(task->status);
  char* cmd = task->cmd.cmdargs[0];
  fprintf(stderr, "[%lu] (%d | %s): %s\n", (unsigned long) task->id,
      (int) task->pid, status, cmd);
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
  size_t i = 0;
  //fprintf(stderr, "TEST1\n");
  while (env->tasks[pos]->cmd.cmdargs[i]) {
    command_t* cmd = &env->tasks[pos]->cmd;
    free(cmd->cmdargs[i++]);
    free(cmd->infile);
    free(cmd->outfile);
  }
  //fprintf(stderr, "TEST2\n");
  free(env->tasks[pos]);
  //fprintf(stderr, "TEST3\n");
  env->tasks_size--;
  env->tasks[pos] = NULL;
}

/**
 * Creates copy of string at heap.
 *
 * @param str String to copy.
 *
 * @return Copied string.
 */
static char* str_realloc(char* str) {
  if (!str)
    return NULL;

  char* copy = (char*) malloc((strlen(str) + 1) * sizeof(char));
  if (!copy)
    return NULL;
  strcpy(copy, str);
  return copy;
}

/**
 * Duplicate the command.
 *
 * @param dest Destination command.
 * @param cmd Command to duplicate
 */
static bool cmddup(command_t* dest, command_t* cmd) {
  if (!cmd || !dest)
    return false;

  dest->flags = cmd->flags;
  dest->infile = str_realloc(cmd->infile);
  dest->outfile = str_realloc(cmd->outfile);

  size_t i = 0;
  while (cmd->cmdargs[i]) {
    dest->cmdargs[i] = str_realloc(cmd->cmdargs[i]);
    if (!(dest->cmdargs[i])) {
      perror("yxsh: Cannot allocate memory");
      while (i)
        free(dest->cmdargs[--i]);
      return false;
    }
    i++;
  }
  dest->cmdargs[i++] = NULL;
  dest->cmdargs[i] = NULL;
  return true;
}

