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
#include <string.h>

#include "builtin.h"
#include "executor.h"

static bool setup_redirects(command_t*);
static bool switch_file_descriptor(int, int, char*);
static bool set_input_file(char*);
static bool set_output_file(char*, int);
static void execute_fork(command_t*, pid_t);
static void execute_parent(tasks_env_t*, pid_t, command_t*);
static pid_t execute_pipeline_command(command_t* cmd, int*, pid_t);
static size_t prepare_pipeline(commandline_t*, size_t);
static void execute_pipeline(tasks_env_t*, commandline_t*, size_t);
static void execute_command(tasks_env_t*, command_t*);

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
  if (cmd->flags & FLAG_IN_PIPE
      && !switch_file_descriptor(cmd->pipes[0], STDIN_FILENO,
        "Cannot redirect input pipe"))
    return false;
  if (cmd->flags & FLAG_OUT_PIPE
      && !switch_file_descriptor(cmd->pipes[1], STDOUT_FILENO,
        "Cannot redirect output pipe"))
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
 * The work is done with a fork process part.
 *
 * @param cmd Command for execution.
 * @param pgid Process group ID.
 */
static void execute_fork(command_t* cmd, pid_t pgid) {
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTSTP, SIG_DFL);
  signal(SIGCHLD, SIG_DFL);
  setup_redirects(cmd);

  setpgid(0, pgid);

  if (!(cmd->flags & (FLAG_BACKGROUND | FLAG_IN_PIPE)) && !cmd->infile) {
    if (!setup_terminal(getpgrp()))
      return;
  }

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
  char* display;
  if (!(cmd->flags & FLAG_BACKGROUND)) {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);
  }

  if (!(display = strdup(cmd->cmdargs[0]))
      || !tasks_has_free(env)
      || !tasks_run_task(env, pid, cmd->flags & FLAG_BACKGROUND, display)) {
    fprintf(stderr, "yxsh: Not enougth space to run task in background.\n");
    if (display)
      free(display);
    killpg(pid, SIGHUP);
    waitpid(pid, NULL, WUNTRACED);
  }

  setup_terminal(getpgrp());
}

/**
 * Executes command normally.
 *
 * @param env Current environment.
 * @param cmd Command.
 */
static void execute_command(tasks_env_t* env, command_t* cmd) {
  pid_t pid = fork();
  switch (pid) {
    case -1:
      perror("yxsh: Cannot create fork");
      return;
    case 0:
      execute_fork(cmd, 0);
      return;
    default:
      setpgid(pid, pid);
      execute_parent(env, pid, cmd);
  }
}

/**
 * Creates pipes and execute fork.
 *
 * @param cmd Command.
 * @param out_pipe Previous output pipe.
 * @param pgid Process group ID.
 *
 * @return 0 if fork process, -1 if error and other if current process.
 */
static pid_t execute_pipeline_command(command_t* cmd, int* out_pipe, pid_t pgid) {
  if (pipe(cmd->pipes)) {
    perror("yxsh: Cannot create pipeline");
    return false;
  }

  pid_t pid = fork();
  switch (pid) {
    case -1:
      perror("yxsh: Cannot create fork");
      break;
    case 0:
      cmd->pipes[0] = (*out_pipe);
      execute_fork(cmd, pgid);
      return pid;
  }

  if (cmd->flags & FLAG_IN_PIPE) {
    close((*out_pipe));
  }
  close(cmd->pipes[1]);
  (*out_pipe) = cmd->pipes[0];

  return pid;
}

/**
 * Sets background flag to all commands in pipeline the same as last command.
 *
 * @param commandline Current command line.
 * @param begin Index of pipeline first command.
 *
 * @return Amount of commands in pipeline.
 */
static size_t prepare_pipeline(commandline_t* commandline, size_t begin) {
  // Search for last pipeline command
  size_t end = begin;
  size_t pos = begin;
  while (end < commandline->ncmds
      && commandline->cmds[++end].flags & (FLAG_IN_PIPE | FLAG_OUT_PIPE));
  end--;

  // Sets background flag for all commands the same as last command
  char bg = (char) (commandline->cmds[end].flags & FLAG_BACKGROUND);
  for (; pos < end; pos++)
    commandline->cmds[pos].flags |= bg;

  return end - begin + 1;
}

/**
 * Executes commands pipeline.
 *
 * @param env Current environment.
 * @param commandline Current command line.
 * @param begin Index of pipeline first command.
 */
static void execute_pipeline(tasks_env_t* env, commandline_t* commandline, size_t begin) {
  size_t pos = begin;
  pid_t pid;
  pid_t main_pid = 0;
  char* display;
  int out_pipe = -1;
  size_t amount = prepare_pipeline(commandline, begin);
  bool bg = commandline->cmds[begin].flags & FLAG_BACKGROUND;
  pid_t* pids = (pid_t*) malloc(sizeof(pid_t) * amount);

  if (!bg) {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);
  }

  while (pos < amount + begin) {
    pid = execute_pipeline_command(&commandline->cmds[pos], &out_pipe, main_pid);
    if (pid == -1 || !pid) {
      if (out_pipe != -1)
        close(out_pipe);
      return;
    }

    if (pos == begin)
      main_pid = pid;
    setpgid(pid, main_pid);
    pids[pos - begin] = pid;
    pos++;
  }

  close(out_pipe);

  display = strdup(commandline->cmds[pos - 1].cmdargs[0]); // TODO - make display string.
  if (!tasks_has_free(env)
      || !tasks_run_pipeline(env, main_pid, pids, bg, amount, display)) {
    fprintf(stderr, "yxsh: Not enougth space to run task in background.\n");
    if (display)
      free(display);
    killpg(pid, SIGHUP);
    waitpid(pid, NULL, WUNTRACED);
  }

  setup_terminal(getpgrp());
}

bool execute(tasks_env_t* env, commandline_t* commandline) {
  for (size_t i = 0; i < commandline->ncmds; i++) {
    command_t* cmd = &commandline->cmds[i];

    int result = try_builtin(env, cmd);
    if (result == BUILTIN_EXECUTED)
      return true;
    if (result == BUILTIN_EXIT)
      return false;

    if (cmd->flags & FLAG_IN_PIPE)
      continue;

    if (cmd->flags & FLAG_OUT_PIPE)
      execute_pipeline(env, commandline, i);
    else
      execute_command(env, cmd);
  }
  return true;
}

