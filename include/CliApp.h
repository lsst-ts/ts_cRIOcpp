/*
 * This file is part of the LSST-TS distribution (https://github.com/lsst-ts).
 * Copyright © 2020 Petr Kubánek, Vera C. Rubin Observatory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CliApp_h
#define __CliApp_h

#include <functional>
#include <fstream>
#include <string>
#include <vector>
#include <list>

typedef std::vector<std::string> command_vec;

struct Argument {
    Argument(char _arg, const char* _help, char _modifer) {
        arg = _arg;
        help = _help;
        modifier = _modifer;
    }
    char arg;
    const char* help;
    char modifier;
};

/**
 * Stores commands and actions for processing.
 *
 * args is argument map. Lowercase characters represents optional arguments,
 * upper case mandatory. The following characters can be used for arguments:
 *
 * ? - variable number of arguments expected
 * Ss - mandatory|optional string argument
 * Ff - mandatory|optional float (double) argument
 * Ii - mandatory|optional integer argument
 */
struct Command {
    Command(const char* _command, std::function<int(command_vec)> _action, const char* _args, int _flags,
            const char* _help_args, const char* _help) {
        command = _command;
        action = _action;
        args = _args;
        flags = _flags;
        help_args = _help_args;
        help = _help;
    }
    const char* command;  // "*" indicates any command (no other command matched)
    std::function<int(command_vec)> action;
    const char* args;
    int flags;
    const char* help_args;
    const char* help;
};

/**
 * Core class for a command line (and interactive) application. Provides
 * functions for parsing command line arguments, using readline and history in
 * the interactive prompt, and providing help for commands.
 *
 * The class shall be value initialized. See the following example code.
 *
 * @code
#include <tcs/utility/CliApp.hpp>
#include <iostream>

AClass : public lbto::CliApp
{
public:
  AClass(const char * description) : CliApp(description), interactive(false) {}
  bool interactive;

protected:
  void printUsage() override;
  void processArg(int opt, const char * optarg) override;
};

void AClass::printUsage()
{
  std::cout << "A simple app. Accept -h for help. Pass -i to start interactive mode." << std::endl;
}

void AClass::processArg(int opt, const char * optarg)
{
  switch (opt)
    {
      case 'h':
        printAppHelp();
        break;
      case 'i':
        interactive = true;
        break;
      default:
        std::cerr << "Unknow command: " << dynamic_cast<char>(opt) << std::endl;
        exit(EXIT_FAILURE);
    }
}

AClass cli("description");

Command commands[] =
{
  {
    "help", [ = ](command_vec cmds) { return cli.helpCommands(cmds); }, "s", 0, "[ALL|command]",
    "Prints all command or command help."
  },
  { NULL, NULL, NULL, 0, NULL, NULL }
};

int main(int argc, char * argv[])
{
  cli.init(commands, "hi", argc, argv);
  if (cli.interactive)
    return goInteractive();
  return 0;
}
 * @endcode
 */
class CliApp {
public:
    /**
     * Construct CliApp.
     *
     * @param _description a short description of the application
     */
    CliApp(const char* description = NULL)
            : verbose(0), progName(NULL), _description(description), _history_fn(NULL) {}

    /**
     * Class destructor. Subclasses are encouraged to include all destruction
     * steps in their own destructor.
     */
    virtual ~CliApp();

    /**
     * Sets CLI description. Description is printed in help text.
     *
     * @param description new description
     */
    void setDescription(const char* description) { _description = description; }

    /**
     * Add argument. Should be called before call to processArgs.
     *
     * @param arg command line argument
     * @param help help string
     * @param modifer optarg modifier - : for required parameter, ? for
     * optional parameter
     */
    void addArgument(const char arg, const char* help, const char modifer = 0);

    void addCommand(const char* command, std::function<int(command_vec)> action, const char* args, int flags,
                    const char* help_args, const char* help);

    /**
     * Initialize the class, parses arguments. Argument parsing stops after the
     * first non-argument. This allows for calls as:
     *
     * app -v -c config.txt command -1.0 -3.141592
     *
     * to pass negative numbers.
     *
     * @param argc argument count (from main method)
     * @param argv argument values (from main method)
     *
     * @return vector with command passed on the command line (after arguments)
     *
     * @see Command
     */
    command_vec processArgs(int argc, char* const argv[]);

    /**
     * Prints application help.
     */
    virtual void printAppHelp();

    /**
     * Print help for a command.
     *
     * @param cmd command for which help will be printed
     */
    void printHelp(const char* cmd);

    /**
     * Prints help for multiple commands.
     */
    int helpCommands(command_vec cmds = std::vector<std::string>());

    /**
     * Starts commands interactive processing.
     */
    void goInteractive(const char* prompt = "> ");

    /**
     * Process character buffer as command.
     */
    int processBuffer(const char* buf);

    /**
     * Reads commands from a file.
     *
     * @param fn filename. Will be opened for reading. Errors will be reported to cerr.
     */
    void readCommands(const char* fn);

    /**
     * Process command entered as command vector.
     *
     * @param cmds command with argument(s)
     *
     * @return 0 on success, error code otherwise
     */
    virtual int processCmdVector(command_vec cmds);

    /**
     * Save command history.
     */
    void saveHistory();

    /**
     * Transforms on/off, 0/1 etc strings into bool.
     */
    static const bool onOff(std::string on);

    int verbose;

protected:
    /**
     * Prints application usage.
     */
    virtual void printUsage() = 0;

    /**
     * Prints preamble for commands help. Can include tips which commands to
     * use,..
     */
    virtual void printGenericHelp(){};

    /**
     * Process option from command line parsing.
     *
     * @param opt option
     * @param optarg option argument, can be null
     *
     * @return -1 on error, 0 on success (shall continue)
     */
    virtual void processArg(int opt, char* optarg) = 0;

    const char* progName;

    /**
     * Can be overwritten to perform any pre/post command processing.
     *
     * @param cmd command to execute
     * @param args command arguments
     *
     * @return 0 on success, -1 on error
     */
    virtual int processCommand(const Command& cmd, const command_vec& args);

    /**
     * Process unmatched commands.
     */
    virtual int processUnmached(command_vec cmds);

    /**
     * List available commands on standard output. Use it inside printUsage().
     */
    void printCommands();

private:
    const char* _description;
    std::list<Argument> _arguments;
    std::list<Command> _commands;
    char* _history_fn;

    /**
     * Find matched command. If multiple commands are matched, returns all possible
     * commands.
     */
    const Command* findCommand(std::string cmd, command_vec& matchedCmds);
    void unknowCommand(std::string cmd, const command_vec matchedCmds);
    void readStreamCommands(std::istream& ins);
    void printCommandHelp(const Command* cmd);
};

#endif  //!
