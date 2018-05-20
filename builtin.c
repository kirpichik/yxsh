//
//  builtin.c
//  yxsh
//
//  Created by Kirill on 11.04.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtin.h"

#define PATH_SIZE 1024

static void change_dirrectory(command_t* cmd) {
  char path[PATH_SIZE];
  char* home;
  size_t home_len, arg_len;

  if (cmd->cmdargs[1] && cmd->cmdargs[2]) {
    fprintf(stderr, "yxsh: cd: to many arguments");
    return;
  }

  if (!(home = getenv("HOME"))) {
    fprintf(stderr, "yxsh: cd: Home path variable is not set.");
    return;
  }

  if (!cmd->cmdargs[1]) { // No args
    if (chdir(home))
      perror("yxsh: cd");
    return;
  }

  home_len = strlen(home);
  arg_len = strlen(cmd->cmdargs[1]);

  if (arg_len + 1 + home_len > PATH_SIZE) {
    fprintf(stderr, "yxsh: cd: To long path.");
    return;
  }

  if (cmd->cmdargs[1][0] == '~') {
    memcpy(path, home, home_len);
    memcpy(path + home_len, cmd->cmdargs[1] + 1, arg_len);
  } else {
    if (strcpy(path, cmd->cmdargs[1]) == NULL) {
      perror("yxsh: cd");
      return;
    }
  }

  if (chdir(path)) {
    perror("yxsh: cd");
  }
}

static task_t* get_task_user(tasks_env_t* env, char* arg) {
  if (!arg) {
    fprintf(stderr, "yxsh: Need argument: <task_id>\n");
    return NULL;
  }
  int id = atoi(arg);
  task_t* task = task_by_id(env, id);
  if (!task) {
    fprintf(stderr, "yxsh: No task with id: %d\n", id);
    return NULL;
  }
  return task;
}

static void jobs_foreground(tasks_env_t* env, command_t* cmd) {
  task_t* task = get_task_user(env, cmd->cmdargs[1]);
  if (!task)
    return;
  task_resume_foreground(env, task);
}

static void jobs_background(tasks_env_t* env, command_t* cmd) {
  task_t* task = get_task_user(env, cmd->cmdargs[1]);
  if (!task)
    return;
  task_resume_background(task);
}

static void jobs_list(tasks_env_t* env) {
  tasks_dump_list(env);
}

int try_builtin(tasks_env_t* env, command_t* cmd) {
  if (!strcmp(cmd->cmdargs[0], "exit")) {
    return BUILTIN_EXIT;
  } else if (!strcmp(cmd->cmdargs[0], "cd")) {
    change_dirrectory(cmd);
  } else if (!strcmp(cmd->cmdargs[0], "fg")) {
    jobs_foreground(env, cmd);
  } else if (!strcmp(cmd->cmdargs[0], "bg")) {
    jobs_background(env, cmd);
  } else if (!strcmp(cmd->cmdargs[0], "jobs")) {
    jobs_list(env);
  } else
    return BUILTIN_NOTFOUND;
  return BUILTIN_EXECUTED;
}

