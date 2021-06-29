/*
 * This file is part of LSST M1M3 support system package.
 *
 * Developed for the LSST Data Management System.
 * This product includes software developed by the LSST Project
 * (https://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "cRIO/CliApp.h"

#include <algorithm>
#include <readline/readline.h>
#include <readline/history.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <libgen.h>
#include <unistd.h>
#include <cmath>
#include <csignal>

namespace LSST {
namespace cRIO {

Command::Command(const char* _command, std::function<int(command_vec)> _action, const char* _args, int _flags,
                 const char* _help_args, const char* _help) {
    command = _command;
    action = _action;
    args = _args;
    flags = _flags;
    help_args = _help_args;
    help = _help;
}

CliApp::~CliApp() {
    if (_history_fn != NULL) {
        saveHistory();

        free(_history_fn);
        _history_fn = NULL;
    }
}

void CliApp::addCommand(const char* command, std::function<int(command_vec)> action, const char* args,
                        int flags, const char* help_args, const char* help) {
    for (auto iter = _commands.begin(); iter != _commands.end(); iter++)
        if (std::string(command) < std::string(iter->command)) {
            _commands.insert(iter, Command(command, action, args, flags, help_args, help));
            return;
        }
    _commands.push_back(Command(command, action, args, flags, help_args, help));
}

void CliApp::printHelp(const char* cmd) {
    command_vec possible;
    const Command* c = findCommand(cmd, possible);

    if (c == NULL) {
        unknowCommand(cmd, possible);
        return;
    }

    printCommandHelp(c);
}

int CliApp::helpCommands(command_vec cmds) {
    if (cmds.empty()) {
        printGenericHelp();
        std::cout << "Commands:" << std::endl;
        printCommands();
        return 0;
    }

    for (auto cm : cmds) {
        if (cm == "all") {
            for (auto c : _commands) {
                printCommandHelp(&c);
            }

            return 0;
        }

        printHelp(cm.c_str());
    }

    return 0;
}

void CliApp::goInteractive(std::string prompt) {
    asprintf(&_history_fn, "%s/.%s_history", getenv("HOME"), getName().c_str());

    using_history();
    int rr = read_history(_history_fn);

    if (rr != ENOENT) {
        if (rr != 0) {
            std::cerr << "Error reading history " << _history_fn << ":" << strerror(rr) << std::endl;
        }

        else if (verbose) {
            std::cout << "Read history from " << _history_fn << std::endl;
        }
    }

    char* buf;

    signal(SIGINT, [](int) { exit(0); });

    while ((buf = readline(prompt.c_str())) != NULL) {
        if (strlen(buf) > 0) {
            add_history(buf);
        }

        processBuffer(buf);
    }

    if (verbose) {
        std::cerr << "Exiting" << std::endl;
    }
}

/**
 * Transform string to lower case.
 */
static inline std::string strlower(const std::string s) {
    std::string ret = s;
    std::transform(ret.begin(), ret.end(), ret.begin(), (int (*)(int))tolower);
    return ret;
}

/**
 * Transform string to upper case.
 */
static inline std::string strupper(const std::string s) {
    std::string ret = s;
    std::transform(ret.begin(), ret.end(), ret.begin(), (int (*)(int))toupper);
    return ret;
}

std::vector<std::string> tokenize(const std::string& input, const std::string& delim = std::string(" ")) {
    std::vector<std::string> vecTokens;

    // Do we have a delimited list?
    if (input.size() > 0) {
        std::string::size_type nStart = 0;
        std::string::size_type nStop = 0;
        std::string::size_type nDelimSize = delim.size();
        std::string szItem;

        // Repeat until we cannot find another delimitor.
        while ((nStop = input.find(delim, nStart)) != std::string::npos) {
            // Pull out the token.
            szItem = input.substr(nStart, nStop - nStart);
            vecTokens.push_back(szItem);
            // Set the new starting position after the delimitor.
            nStart = nStop + nDelimSize;
        }
        // Are there any chars left after the last delim?
        if (nStop == std::string::npos && nStart < input.size()) {
            // There are chars after the last delim - this is the last token.
            szItem = input.substr(nStart, input.size() - nStart);
            vecTokens.push_back(szItem);
        }
    }
    return vecTokens;
}

int CliApp::processBuffer(const char* buf) {
    command_vec cmds = tokenize(buf);

    if (cmds.empty()) {
        return -1;
    }

    return processCmdVector(cmds);
}

void CliApp::readCommands(const char* fn) {
    if (strcmp(fn, "-")) {
        std::ifstream ins(fn);

        if (!ins) {
            std::cerr << "Cannot open " << fn << ": " << strerror(errno) << std::endl;
            return;
        }

        readStreamCommands(ins);
    }

    else {
        readStreamCommands(std::cin);
    }
}

int CliApp::processCmdVector(command_vec cmds) {
    std::string cmd = strlower(cmds.front());

    command_vec matchedCmds;

    Command* c = findCommand(cmd, matchedCmds);

    if (c == NULL) {
        if (matchedCmds.empty()) {
            return processUnmached(cmds);
        }

        else {
            std::cerr << "multiple commands matching " << cmd << ":";

            for (auto c : matchedCmds) {
                std::cerr << " " << c;
            }

            std::cerr << std::endl;
        }

        return -1;
    }

    cmds.erase(cmds.begin());
    return processCommand(c, cmds);
}

void CliApp::saveHistory() {
    if (_history_fn != NULL) {
        int wr = write_history(_history_fn);

        switch (wr) {
            case 0:
                if (verbose) {
                    std::cout << "History saved to " << _history_fn << std::endl;
                }

                break;

            default:
                std::cerr << "Unable to save history to " << _history_fn << ":" << strerror(wr) << std::endl;
        }
    }
}

const bool CliApp::onOff(std::string on) {
    if (strcasecmp(on.c_str(), "on") == 0 || on == "1") return true;
    if (strcasecmp(on.c_str(), "off") == 0 || on == "0") return false;
    throw std::runtime_error("Invalid on/off string:" + on);
}

const void CliApp::printHexBuf(uint8_t* buf, size_t len, const char* prefix) {
    std::cout << std::hex;
    for (size_t i = 0; i < len; i++) {
        std::cout << prefix << std::setfill('0') << std::setw(2) << static_cast<int>(buf[i]);
    }
    std::cout << std::dec;
}

/**
 * Verify if passed arguments match argument map.
 *
 * Returns >= 0 as number of matching arguments. Print error message and returns
 * -1 if there isn't a match.
 */
int verifyArguments(const command_vec& cmds, const char* args) {
    auto verifyDouble = [](const char* d) -> int {
        try {
            std::stod(d);
            return true;
        } catch (...) {
            return false;
        }
    };

    auto verifyInteger = [](const char* i) -> int {
        try {
            std::stoi(i);
            return true;
        } catch (...) {
            return false;
        }
    };

    auto verifyBool = [](const char* b) -> int {
        std::string s = strupper(b);
        return (s == "TRUE" || s == "FALSE");
    };

    size_t an = 0;

    for (const char* a = args; *a; a++, an++) {
        if (an >= cmds.size()) {
            if (*a == 's' || *a == 'i' || *a == 'b' || *a == 'd' || *a == 'h' || *a == '?') {
                return an;
            }

            std::cerr << "Required arguments are missing, expected at least " << (an + 1) << ", got "
                      << cmds.size() << std::endl;
            return -1;
        }

        switch (*a) {
            case '?':
                return cmds.size();
            case 'D':
            case 'd':
                if (!verifyDouble(cmds[an].c_str())) {
                    std::cerr << "Expecting double number, received " << cmds[an] << std::endl;
                    return -1;
                }

                break;

            case 'F': {
                struct stat fsta;
                if (stat(cmds[an].c_str(), &fsta) != 0) {
                    std::cerr << "Unable to access file " << cmds[an] << ": " << strerror(errno) << std::endl;
                    return -1;
                }
                break;
            }

            case 'I':
            case 'i':
                if (!verifyInteger(cmds[an].c_str())) {
                    std::cerr << "Expecting integer number, received " << cmds[an] << std::endl;
                    return -1;
                }

                break;

            case 'B':
            case 'b':
                if (!verifyBool(cmds[an].c_str())) {
                    std::cerr << "Expecting boolean (true/false), received " << cmds[an] << std::endl;
                    return -1;
                }

                break;

            case 'S':
            case 's':
                break;

            default:
                std::cerr << "Invalid formatting character " << *a << std::endl;
                return -1;
        }
    }

    return an;
}

int CliApp::processCommand(Command* cmd, const command_vec& args) {
    try {
        if (verifyArguments(args, cmd->args) >= 0) {
            return cmd->action(args);
        }
    }

    catch (std::exception& err) {
        std::cerr << "Processing " << cmd->command;

        for (auto ar : args) {
            std::cerr << " " << ar;
        }

        std::cerr << ": " << err.what() << std::endl;
    }

    return -1;
}

int CliApp::processUnmached(command_vec cmds) {
    std::cerr << "Unknown command: " << cmds.front() << std::endl;
    return -1;
}

void CliApp::printCommands() {
    for (auto command : _commands) {
        std::cout << " " << command.command << std::endl;
    }
}

Command* CliApp::findCommand(std::string cmd, command_vec& matchedCmds) {
    Command* ret = NULL;

    for (std::list<Command>::iterator tc = _commands.begin(); tc != _commands.end(); tc++) {
        if (strncmp(cmd.c_str(), tc->command, cmd.length()) == 0) {
            matchedCmds.push_back(tc->command);
            ret = &(*tc);
        }
    }

    if (matchedCmds.size() != 1) {
        return NULL;
    }

    return ret;
}

void CliApp::unknowCommand(std::string cmd, const command_vec matchedCmds) {
    if (matchedCmds.empty()) {
        std::cerr << "Unknow command " << cmd << ", please see help for allowed commands." << std::endl;
        return;
    }

    std::cerr << "Possible matches:";

    for (auto cm : matchedCmds) {
        std::cerr << " " << cm;
    }

    std::cerr << std::endl;
}

void CliApp::readStreamCommands(std::istream& ins) {
    while (!ins.eof()) {
        char cmdbuf[500];
        memset(cmdbuf, 0, sizeof(cmdbuf));
        ins.getline(cmdbuf, sizeof(cmdbuf));

        if (cmdbuf[0] == 0 and ins.eof()) {
            break;
        }

        if (cmdbuf[0] == '#' || cmdbuf[0] == '\0') {
            continue;
        }

        processBuffer(cmdbuf);
    }
}

void CliApp::printCommandHelp(const Command* cmd) {
    std::cout << std::endl << " * " << strupper(cmd->command) << std::endl << std::endl;

    if (cmd->help_args) {
        std::cout << cmd->help_args << std::endl << std::endl;
    }

    std::cout << cmd->help << std::endl << std::endl;
}

}  // namespace cRIO
}  // namespace LSST
