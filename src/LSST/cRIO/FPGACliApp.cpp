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

#include "cRIO/FPGACliApp.h"

#include <iostream>

using namespace LSST::cRIO;

FPGACliApp::FPGACliApp(const char* description) : CliApp(description), _fpga(nullptr), _autoOpen(true) {
    addArgument('d', "increase debug level");
    addArgument('h', "print this help");
    addArgument('O', "don't auto open (and run) FPGA");

    addCommand("close", std::bind(&FPGACliApp::closeFPGA, this, std::placeholders::_1), "", NEED_FPGA, NULL,
               "Close FPGA connection");
    addCommand("help", std::bind(&FPGACliApp::helpCommands, this, std::placeholders::_1), "", 0, NULL,
               "Print commands help");
    addCommand("open", std::bind(&FPGACliApp::openFPGA, this, std::placeholders::_1), "", 0, NULL,
               "Open FPGA");
    addCommand("verbose", std::bind(&FPGACliApp::verbose, this, std::placeholders::_1), "?", 0, "<new level>",
               "Report/set verbosity level");
}

FPGACliApp::~FPGACliApp() {}

int FPGACliApp::info(command_vec cmds) {
    _clearILCs();

    ILCUnits units = getILCs(cmds);

    if (units.empty()) {
        return -1;
    }

    for (auto u : units) {
        u.first->reportServerID(u.second);
    }

    for (auto ilcp : ilcs) {
        if (ilcp->getLength() > 0) {
            _fpga->ilcCommands(*ilcp);
        }
    }

    return 0;
}

int FPGACliApp::openFPGA(command_vec cmds) {
    if (_fpga != nullptr) {
        std::cerr << "FPGA already opened!" << std::endl;
        return 1;
    }
    char dir[255];
    if (cmds.size() == 0) {
        getcwd(dir, 255);
    } else {
        memcpy(dir, cmds[0].c_str(), cmds[0].length() + 1);
    }
    _fpga = newFPGA(dir);
    _fpga->initialize();
    _fpga->open();
    return 0;
}

int FPGACliApp::closeFPGA(command_vec cmds) {
    _fpga->close();
    delete _fpga;
    _fpga = nullptr;
    return 0;
}

int FPGACliApp::verbose(command_vec cmds) {
    switch (cmds.size()) {
        case 1:
            setDebugLevel(std::stoi(cmds[0]));
        case 0:
            std::cout << "Debug level: " << getDebugLevel() << std::endl;
            break;
    }
    return 0;
}

void FPGACliApp::processArg(int opt, char* optarg) {
    switch (opt) {
        case 'd':
            incDebugLevel();
            break;

        case 'h':
            printAppHelp();
            exit(EXIT_SUCCESS);
            break;

        case 'O':
            _autoOpen = false;
            break;

        default:
            std::cerr << "Unknown command: " << (char)(opt) << std::endl;
            exit(EXIT_FAILURE);
    }
}

int FPGACliApp::processCommand(Command* cmd, const command_vec& args) {
    if ((cmd->flags & NEED_FPGA) && _fpga == nullptr) {
        std::cerr << "Command " << cmd->command << " needs opened FPGA. Please call open command first"
                  << std::endl;
        return -1;
    }
    return CliApp::processCommand(cmd, args);
}
