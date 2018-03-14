//
//  command.h
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#ifndef _COMMAND_H
#define _COMMAND_H

#include <string>
#include <vector>

/**
 * Single parsed command.
 */
class Command {
 public:
  Command(const std::vector<std::string>& args,
          const bool bkg,
          const std::string& in,
          const std::string& out,
          const std::string& app)
      : arguments(args),
        background(bkg),
        infile(in),
        outfile(out),
        appfile(app) {}

  const std::string& operator[](int pos) const { return arguments[pos]; }

  size_t argsSize() const { return arguments.size(); }

  bool isBackground() const { return background; }

  bool hasInputFile() const { return !infile.empty(); }

  bool hasOutputFile() const { return !outfile.empty(); }

  bool hasApplicableFile() const { return !appfile.empty(); }

  const std::string& getInputFile() const { return infile; }

  const std::string& getOutputFile() const { return outfile; }

  const std::string& getApplicableFile() const { return appfile; }

 private:
  const std::vector<std::string> arguments;
  const bool background;
  const std::string infile;
  const std::string outfile;
  const std::string appfile;
};

/**
 * Ordered set of commands.
 */
class CommandsSet {
 public:
  /**
   * Parse error constructor.
   */
  CommandsSet() : valid(false) {}

  /**
   * Parse OK constructor.
   */
  CommandsSet(const std::vector<Command>& cmds) : valid(true), commands(cmds) {}

  /**
   * @return Is prompt line parsed without syntax errors.
   */
  bool isValid() { return valid; }

  const Command& operator[](int pos) const { return commands[pos]; }

  size_t commandsCount() { return commands.size(); }

 private:
  const bool valid;
  const std::vector<Command> commands;
};

#endif /* _COMMAND_H */
