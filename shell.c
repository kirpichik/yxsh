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
#include <readline/readline.h>
#include <readline/history.h>

#include "executor.h"
#include "parseline.h"
#include "shell.h"
#include "tasks.h"

#define INPUT_BUFF 1024

#define PROMPT_PREFIX "[yx!>"
#define PROMPT_SUFFIX "]> "

static bool form_prompt(char*);
static void print_prompt();
static void child_update_signal_handler(int);
static void resetup_signals();
static void shell_add_history(char*);

static tasks_env_t environment;

int main(int argc, char* argv[]) {
  commandline_t commandline;
  char prompt[INPUT_BUFF];
  char* line;
  int ncmds;

  tasks_create_env(&environment);
  resetup_signals();

  while (form_prompt(prompt) && (line = readline(prompt)) != NULL) {
    if ((ncmds = parseline(line, &commandline)) > 0) {
      shell_add_history(line);
      if (!execute(&environment, &commandline)) {
        free_cmds_strings(&commandline, ncmds);
        tasks_collect_zombies(&environment);
        tasks_release_env(&environment);
        free(line);
        clear_history();
        exit(0);
      }
      free_cmds_strings(&commandline, ncmds);
    }
    free(line);
    tasks_collect_zombies(&environment);
    resetup_signals();
  }

  return 0;
}

static void shell_add_history(char* cmd) {
  HIST_ENTRY* tmp = previous_history();
  if (!tmp) {
    add_history(cmd);
    return;
  }

  if (!strcmp(tmp->line, cmd)) {
    tmp = remove_history(where_history());
    free_history_entry(tmp);
  }

  add_history(cmd);
}

static void resetup_signals() {
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGCHLD, &child_update_signal_handler);
}

static bool form_prompt(char* prompt) {
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

  if (snprintf(prompt, INPUT_BUFF, "%s%s%s", PROMPT_PREFIX, path, PROMPT_SUFFIX) <= 0) {
    fprintf(stderr, "yxsh: Cannot form prompt.");
    return false;
  }

  return true;
}

static void print_prompt() {
  char prompt[INPUT_BUFF];
  if (!form_prompt(prompt))
    exit(-1);
  write(STDOUT_FILENO, prompt, strlen(prompt));
}

static void child_update_signal_handler(int sig) {
  if (tasks_update_status(&environment))
    print_prompt();
}

