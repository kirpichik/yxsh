//
//  shell.c
//  yxsh
//
//  Created by Kirill on 14.03.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "executor.h"
#include "parseline.h"
#include "promptline.h"
#include "shell.h"
#include "tasks.h"

#define PROMPT_PREFIX "[yx!>"
#define PROMPT_SUFFIX "]> "
#define INPUT_BUFF 1024

static bool print_prompt();
static void child_update_signal_handler(int);
static void exit_signal_handler(int);
static void resetup_signals();

static tasks_env_t environment;

int main(int argc, char* argv[]) {
  commandline_t commandline;
  char line[INPUT_BUFF];
  int ncmds;

  tasks_create_env(&environment);
  resetup_signals();
  signal(SIGCHLD, &child_update_signal_handler);

  while (print_prompt() && promptline(line, sizeof(line)) > 0) {
    if ((ncmds = parseline(line, &commandline)) > 0) {
      execute(&environment, &commandline, ncmds);
      free_cmds_strings(&commandline, ncmds);
    }
    resetup_signals();
  }

  return 0;
}

static void resetup_signals() {
  signal(SIGINT, &exit_signal_handler);
  signal(SIGTSTP, &exit_signal_handler);
  signal(SIGQUIT, &exit_signal_handler);
}

static bool print_prompt() {
  char path[INPUT_BUFF];
  char* home;

  if (getcwd(path, sizeof(path)) == NULL) {
    perror("yxsh: Cannot read current path");
    return false;
  }

  if (!(home = getenv("HOME"))) {
    fprintf(stderr, "yxsh: Home path variable is not set.");
    return false;
  }

  if (!strncmp(path, home, strlen(home))) {
    memcpy(path + 1, path + strlen(home), strlen(path) - strlen(home) + 1);
    path[0] = '~';
  }

  write(STDOUT_FILENO, PROMPT_PREFIX, strlen(PROMPT_PREFIX));
  write(STDOUT_FILENO, path, strlen(path));
  write(STDOUT_FILENO, PROMPT_SUFFIX, strlen(PROMPT_SUFFIX));

  return true;
}

static void child_update_signal_handler(int sig) {
  int status;
  pid_t pid = wait(&status);
  fprintf(stderr, "\n");
  tasks_update_status(&environment, pid, status);
  print_prompt();
}

static void exit_signal_handler(int sig) {
  write(STDOUT_FILENO, "\n", 1);
  if (!print_prompt())
    exit(-1);
}

