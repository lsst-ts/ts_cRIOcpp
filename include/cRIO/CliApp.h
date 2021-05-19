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

#include <cRIO/Application.h>

namespace LSST {
namespace cRIO {

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
            const char* _help_args, const char* _help);
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
#include <cRIO/CliApp.h>
#include <iostream>

using namespace LSST::cRIO;

class AClass : public CliApp {
public:
    AClass(const char* description) : CliApp(description), interactive(false) {}
    bool interactive;

protected:
    void printUsage() override;
    void processArg(int opt, char* optarg) override;
};

void AClass::printUsage() {
    std::cout << "A simple app. Accept -h for help. Pass -i to start interactive mode." << std::endl;
}

void AClass::processArg(int opt, char* optarg) {
    switch (opt) {
        case 'h':
            printAppHelp();
            break;
        case 'i':
            interactive = true;
            break;
        default:
            std::cerr << "Unknow command: " << static_cast<char>(opt) << std::endl;
            exit(EXIT_FAILURE);
    }
}

int main(int argc, char* const[] argc) {
    AClass cli("description");

    cli.addCommand("help", [&cli](command_vec cmds) { return cli.helpCommands(cmds); }, "s", 0,
                   "[ALL|command]", "Prints all command or command help.");

    command_vec cmds = cli.processArgs(argc, argv);
    cli.processCmdVector(cmds);
    if (cli.interactive) return cli.goInteractive();
    return 0;
}
 * @endcode
 */
class CliApp : public Application {
public:
    /**
     * Construct CliApp.
     *
     * @param _description a short description of the application
     */
    CliApp(const char* description) : Application(description), _history_fn(NULL) {}

    /**
     * Class destructor. Subclasses are encouraged to include all destruction
     * steps in their own destructor.
     */
    virtual ~CliApp();

    void addCommand(const char* command, std::function<int(command_vec)> action, const char* args, int flags,
                    const char* help_args, const char* help);

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

protected:
    /**
     * Can be overwritten to perform any pre/post command processing.
     *
     * @param cmd command to execute
     * @param args command arguments
     *
     * @return 0 on success, -1 on error
     */
    virtual int processCommand(Command* cmd, const command_vec& args);

    /**
     * Process unmatched commands.
     */
    virtual int processUnmached(command_vec cmds);

    /**
     * List available commands on standard output. Use it inside printUsage().
     */
    void printCommands();

    int verbose;

private:
    std::list<Command> _commands;
    char* _history_fn;

    /**
     * Find matched command. If multiple commands are matched, returns all possible
     * commands.
     */
    Command* findCommand(std::string cmd, command_vec& matchedCmds);
    void unknowCommand(std::string cmd, const command_vec matchedCmds);
    void readStreamCommands(std::istream& ins);
    void printCommandHelp(const Command* cmd);
};

}  // namespace cRIO
}  // namespace LSST

#endif  //!
