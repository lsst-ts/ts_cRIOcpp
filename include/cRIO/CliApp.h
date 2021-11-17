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
 * Dd - mandatory|optional double (float) argument
 * F - mandatory filename argument
 * Ii - mandatory|optional integer argument
 * Ss - mandatory|optional string argument
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
 * Use Application::addArgument to add arguments and CliApp::addCommand to add
 * commands.
 *
 * See the following example code for usage.
 *
 * @code
#include <cRIO/CliApp.h>
#include <iostream>

using namespace LSST::cRIO;

class AClass : public CliApp {
public:
    AClass() : CliApp("AnApp", "demo CliApp subclass"), interactive(false) {}
    bool interactive;

protected:
    void printUsage() override;
    void processArg(int opt, char* optarg) override;
};

void AClass::printUsage() {
    std::cout << "A simple app. Accept -h for help. Pass -i to start interactive mode." << std::endl;
    CliApp::printUsage();
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

    cli.addArgument('h', "print help");
    cli.addArgument('i', "starts interactive mode");

    cli.addCommand("help", std::bind(&AClass::helpCommands, &cli, std::placeholders::_1), "s", 0,
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
    CliApp(const char* name, const char* description) : Application(name, description), _history_fn(NULL) {}

    /**
     * Class destructor. Subclasses are encouraged to include all destruction
     * steps in their own destructor.
     */
    virtual ~CliApp();

    /**
     * Add command for processing.
     *
     * @section Arguments characters
     *
     * The following characters can occur in args strings. Lower case are
     * optional (argument can occur), upper case means required (argument must
     * be provided).
     *
     * Character | Description
     * --------- | -----------
     * bB        | Boolean value. TRUE, FALSE, 0, 1, ON, OFF (case insensitive) are supported
     * dD        | Double (float) value. Any notation shall be supported (scientific, fixed, ..)
     * hH        | Hex value. Numbers and [A-F] are allowed.
     * iI        | Integer. Standard 0x prefixes can be used to signal binary, hex, ..
     * sS        | String.
     * ?         | The argument can occur multiple times. Shall occur only once, at the end of the argument
specification.
     *
     * @param command the command
     * @param action action triggered
     * @param args allowed arguments
     * @param flags custom flags. Up to child class which flags will be supported
     * @param help_args arguments list printed in help
     * @param help help content - describe command
     *
     * @see processArg
     * @see processCommand
     *
     * @example
     *
     * Assuming cli is address of some CliApp class, call the following to bind call of theAnswer method to
get_the_answer command.
     *
     * @code
    addCommand("get_the_answer", std::bind(&Class::theAnswer, &cli, std::placeholders::_1), "s?", 0,
               "[choice..]", "Get random answer.");
     * @endcode
     */
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
    void goInteractive(std::string prompt = "> ");

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

    /**
     * Utitlity function to print out buffer as hex dump.
     *
     * @param buf buffer to print
     * @param len length of the buffer
     */
    static const void printHexBuf(uint8_t* buf, size_t len, const char* prefix = "");

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
