//
//  executor.c
//  yxsh
//
//  Created by Кирилл on 02.04.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
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

#include "executor.h"
#include "builtin.h"

static bool set_input_file(char*);
static bool set_output_file(char*, int);

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
  bool result;
  int flags = O_WRONLY;

  if (redirects & FLAG_APPLY_FILE)
    flags |= O_CREAT | O_APPEND;
  else
    flags |= O_CREAT | O_TRUNC;

  if ((file = open(outfile, flags, (mode_t)0644)) == -1) {
    perror("yxsh: Cannot open output file");
    return false;
  }

  printf("MERGE: %d\n", redirects & FLAG_APPLY_FILE);

  result = switch_file_descriptor(file, STDOUT_FILENO,
      "yxsh: Cannot set output file");
  if (!result)
    return false;

  if (redirects & FLAG_MERGE_OUT) {
    if ((file = open(outfile, flags, (mode_t)0644)) == -1) {
      perror("yxsh: Cannot open output file");
      return false;
    }

    return switch_file_descriptor(file, STDERR_FILENO, 
        "yxsh: Cannot set output file");
  }
  return result;
}

/**
 * The work is done with a fork process part.
 *
 * @param cmd Command for execution.
 */
static void execute_fork(command_t* cmd) {
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
static void execute_parent(pid_t pid, command_t* cmd) {
  if (cmd->flags & FLAG_BACKGROUND) {
    printf("yxsh: Running background: %d\n", (int)pid);
    return;
  }

  int status = 0;
  pid_t result = waitpid(pid, &status, WUNTRACED);
  if (result == -1)
    perror("Cannot wait for child process termination");
}

void execute(commandline_t* commandline, size_t ncmds) {
  for (size_t i = 0; i < ncmds; i++) {
    
    if (try_builtin(&commandline->cmds[i]))
      return;

    pid_t pid = fork();
    switch (pid) {
      case -1:
        perror("Cannot create process");
        return;
      case 0:
        execute_fork(&commandline->cmds[i]);
        return;
      default:
        execute_parent(pid, &commandline->cmds[i]);
    }
  }
}

