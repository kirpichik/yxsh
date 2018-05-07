//
//  executor.c
//  yxsh
//
//  Created by Kirill on 02.04.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "builtin.h"
#include "executor.h"

static bool setup_redirects(command_t*);
static bool switch_file_descriptor(int, int, char*);
static bool set_input_file(char*);
static bool set_output_file(char*, int);
static void execute_fork(command_t*);
static void execute_parent(tasks_env_t*, pid_t, command_t*);
static void setup_fork_signals(bool);

/**
 * Setup input/output redirects if presented.
 *
 * @param cmd Target command.
 */
static bool setup_redirects(command_t* cmd) {
  if (cmd->infile && !set_input_file(cmd->infile))
    return false;
  if (cmd->outfile && !set_output_file(cmd->outfile, cmd->flags))
    return false;
  return true;
}

/**
 * Switches file descriptors using dup2(2) function.
 *
 * @param oldfd Old file descriptor.
 * @param newfd Purpose to copy descriptor.
 * @param err Error message if it occurred.
 *
 * @return true if success.
 */
static bool switch_file_descriptor(int oldfd, int newfd, char* err) {
  if (dup2(oldfd, newfd) == -1) {
    perror(err);
    close(oldfd);
    return false;
  }

  close(oldfd);
  return true;
}

/**
 * Redirects input from file to STDIN descriptor.
 *
 * @param infile Input file name.
 *
 * @return true if success.
 */
static bool set_input_file(char* infile) {
  int file = open(infile, O_RDONLY);
  if (file == -1) {
    perror("yxsh: Cannot open input file");
    return false;
  }

  return switch_file_descriptor(file, STDIN_FILENO,
                                "yxsh: Cannot set input file");
}

/**
 * Redirects output from STDOUT to file.
 *
 * @param outfile Output file name.
 * @param redirects Flags for redirects.
 *
 * @return true if success.
 */
static bool set_output_file(char* outfile, int redirects) {
  int file;
  int flags = O_WRONLY | O_CREAT;

  if (redirects & FLAG_MERGE_OUT) {
    flags |= O_APPEND;
    if ((file = open(outfile, flags, (mode_t)0644)) == -1) {
      perror("yxsh: Cannot open err output file");
      return false;
    }
    if (!switch_file_descriptor(file, STDERR_FILENO,
                                "yxsh: Cannot set stderr file"))
      return false;
  } else if (redirects & FLAG_APPLY_FILE)
    flags |= O_APPEND;
  else
    flags |= O_CREAT | O_TRUNC;

  if ((file = open(outfile, flags, (mode_t)0644)) == -1) {
    perror("yxsh: Cannot open output file");
    return false;
  }

  return switch_file_descriptor(file, STDOUT_FILENO,
                                "yxsh: Cannot set output file");
}

/**
 * Sets signals handlers for executing command.
 *
 * @param background Is command runnig in background.
 */
static void setup_fork_signals(bool background) {
  if (background) {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
  } else {
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
  }

  signal(SIGTSTP, SIG_DFL);
}

/**
 * The work is done with a fork process part.
 *
 * @param cmd Command for execution.
 */
static void execute_fork(command_t* cmd) {
  setup_fork_signals(cmd->flags & FLAG_BACKGROUND);
  setup_redirects(cmd);
  if (execvp(cmd->cmdargs[0], cmd->cmdargs))
    perror("yxsh: Cannot execute");
  exit(0);
}

/**
 * The work is done with a parent process part.
 *
 * @param pid Process ID of sub process.
 * @param cmd Command for execution.
 */
static void execute_parent(tasks_env_t* env, pid_t pid, command_t* cmd) {
  int status;
  if (cmd->flags & FLAG_BACKGROUND) {
    if (!tasks_create_task(pid, cmd, env)) // TODO - kill or foreground task?
      fprintf(stderr, "yxsh: Not enougth space to run task in background.\n");
    return;
  }

  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);

  if (waitpid(pid, &status, WUNTRACED) != -1) {
    if (WIFSTOPPED(status)) { // TODO - recatch of STOPPED
      if (!tasks_create_task(pid, cmd, env)) {
        fprintf(stderr, "yxsh: Not enougth space to run task in background.\n");
        // TODO - kill task or return foreground.
      }
      return;
    }
  } else {
    perror("yxsh: Cannot wait for child process termination");
    return;
  }
}

void execute(tasks_env_t* env, commandline_t* commandline, size_t ncmds) {
  for (size_t i = 0; i < ncmds; i++) {
    if (try_builtin(env, &commandline->cmds[i]))
      return;

    pid_t pid = fork();
    switch (pid) {
      case -1:
        perror("yxsh: Cannot create fork");
        return;
      case 0:
        execute_fork(&commandline->cmds[i]);
        return;
      default:
        execute_parent(env, pid, &commandline->cmds[i]);
    }
  }
}

