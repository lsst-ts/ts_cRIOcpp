/*
 * This file is part of LSST M1M3 support system package.
 *
 * Developed for the Vera C. Rubin Telescope and Site System.
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

#include <iomanip>
#include <iostream>

#include "cRIO/FPGACliApp.h"
#include "cRIO/IntelHex.h"

using namespace LSST::cRIO;

std::ostream& LSST::cRIO::operator<<(std::ostream& stream, ILCUnit const& u) {
    stream << static_cast<int>(u.first->getBus()) << "/" << static_cast<int>(u.second);
    return stream;
}

FPGACliApp::FPGACliApp(const char* name, const char* description)
        : CliApp(name, description), _fpga(nullptr), _ilcs(), _autoOpen(true), _timeIt(false) {
    addArgument('d', "increase debug level");
    addArgument('h', "print this help");
    addArgument('O', "don't auto open (and run) FPGA");

    addCommand("@timeit", std::bind(&FPGACliApp::timeit, this, std::placeholders::_1), "b", 0, "[flag]",
               "Sets timing flag");
    addCommand("close", std::bind(&FPGACliApp::closeFPGA, this, std::placeholders::_1), "", NEED_FPGA, NULL,
               "Close FPGA connection");

    addILCCommand("@disable", std::bind(&FPGACliApp::disableILC, this, std::placeholders::_1),
                  "Temporary disable given ILC in * commands");
    addILCCommand("@enable", std::bind(&FPGACliApp::enableILC, this, std::placeholders::_1),
                  "Re-enable given ILC in * commands");
    addILCCommand(
            "info", [](ILCUnit u) { u.first->reportServerID(u.second); }, "Print ILC info");
    addILCCommand(
            "status", [](ILCUnit u) { u.first->reportServerStatus(u.second); }, "Print ILC status");
    addILCCommand(
            "standby", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::ILCMode::Standby); },
            "Change to standby mode");
    addILCCommand(
            "disable", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::ILCMode::Disabled); },
            "Change to disabled mode");
    addILCCommand(
            "enable", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::ILCMode::Enabled); },
            "Change to enabled mode");
    addILCCommand(
            "clear-faults", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::ILCMode::ClearFaults); },
            "Clear ILC faults");
    addILCCommand(
            "reset", [](ILCUnit u) { u.first->resetServer(u.second); }, "Reset server");

    addCommand("program-ilc", std::bind(&FPGACliApp::programILC, this, std::placeholders::_1), "FS?",
               NEED_FPGA, "<firmware hex file> <ILC...>", "Program ILC with new firmware.");
    addCommand("help", std::bind(&FPGACliApp::helpCommands, this, std::placeholders::_1), "s", 0, "[command]",
               "Print commands help");
    addCommand("open", std::bind(&FPGACliApp::openFPGA, this, std::placeholders::_1), "", 0, NULL,
               "Open FPGA");
    addCommand("verbose", std::bind(&FPGACliApp::verbose, this, std::placeholders::_1), "?", 0, "<new level>",
               "Report/set verbosity level");
}

FPGACliApp::~FPGACliApp() {}

int FPGACliApp::run(int argc, char* const argv[]) {
    command_vec cmds = processArgs(argc, argv);

    if (_autoOpen) {
        command_vec cmds;
        openFPGA(cmds);
    }

    if (cmds.empty()) {
        std::cout << "Please type help for more help." << std::endl;
        goInteractive(getName() + " > ");
        closeFPGA(command_vec());
        return 0;
    }

    return processCmdVector(cmds);
}

int FPGACliApp::timeit(command_vec cmds) {
    if (cmds.size() == 1) {
        _timeIt = onOff(cmds[0]);
    }
    if (_timeIt) {
        std::cout << "Will time executed commands." << std::endl;
    } else {
        std::cout << "Commands will not be timed." << std::endl;
    }
    return 0;
}

int FPGACliApp::closeFPGA(command_vec cmds) {
    _fpga->close();
    delete _fpga;
    _fpga = nullptr;
    return 0;
}

int FPGACliApp::programILC(command_vec cmds) {
    IntelHex hf;
    hf.load(cmds[0]);

    cmds.erase(cmds.begin());
    ILCUnits units = getILCs(cmds);

    if (units.empty()) {
        return -1;
    }

    for (auto u : units) {
        u.first->programILC(getFPGA(), u.second, hf);
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

void FPGACliApp::addILCCommand(const char* command, std::function<void(ILCUnit)> action, const char* help) {
    bool disableDisabled = strcmp(command, "@enable") == 0;
    addCommand(
            command,
            [action, disableDisabled, this](command_vec cmds) -> int {
                clearILCs();

                ILCUnits units = getILCs(cmds);

                if (units.empty()) {
                    return -1;
                }

                for (auto u : units) {
                    auto it = std::find(_disabledILCs.begin(), _disabledILCs.end(), u);
                    if (it == _disabledILCs.end() || disableDisabled) {
                        action(u);
                    } else {
                        std::cout << "ILC " << u << " disabled." << std::endl;
                    }
                }

                for (auto ilcp : _ilcs) {
                    if (ilcp->getLength() > 0) {
                        _fpga->ilcCommands(*ilcp);
                    }
                }

                return 0;
            },
            "s?", NEED_FPGA, "<address>...", help);
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
            std::cerr << "Unknown argument: " << (char)(opt) << std::endl;
            exit(EXIT_FAILURE);
    }
}

int FPGACliApp::processCommand(Command* cmd, const command_vec& args) {
    if ((cmd->flags & NEED_FPGA) && _fpga == nullptr) {
        std::cerr << "Command " << cmd->command << " needs opened FPGA. Please call open command first"
                  << std::endl;
        return -1;
    }

    auto start = std::chrono::steady_clock::now();

    int ret = CliApp::processCommand(cmd, args);

    auto end = std::chrono::steady_clock::now();

    if (_timeIt) {
        std::chrono::duration<double> diff = end - start;
        std::cout << "Took " << std::setprecision(3) << (diff.count() * 1000.0) << " ms" << std::endl;
    }

    return ret;
}

std::shared_ptr<MPU> FPGACliApp::getMPU(std::string name) {
    std::shared_ptr<MPU> ret = NULL;
    for (auto m : _mpu) {
        if (strncmp(name.c_str(), m.first.c_str(), name.length()) == 0) {
            if (ret) {
                return NULL;
            }
            ret = m.second;
        }
    }
    return ret;
}

void FPGACliApp::disableILC(ILCUnit u) {
    _disabledILCs.push_back(u);
    printDisabled();
}

void FPGACliApp::enableILC(ILCUnit u) {
    auto it = std::find(_disabledILCs.begin(), _disabledILCs.end(), u);
    if (it == _disabledILCs.end()) {
        std::cerr << "No such ILC: " << u << std::endl;
        return;
    }
    _disabledILCs.erase(it);
    printDisabled();
}

void FPGACliApp::printDisabled() {
    if (_disabledILCs.empty()) {
        std::cout << "All ILC enabled." << std::endl;
        return;
    }
    std::cout << "Disabled ILC" << (_disabledILCs.size() > 1 ? "s: " : ": ");
    size_t i = 0;
    for (auto it = _disabledILCs.begin(); it != _disabledILCs.end(); it++, i++) {
        if (it != _disabledILCs.begin()) std::cout << (i == _disabledILCs.size() - 1 ? " and " : ", ");
        std::cout << *it;
    }
    std::cout << "." << std::endl;
}

void FPGACliApp::printMPU() {
    for (auto m : _mpu) {
        std::cerr << "  * " << m.first << std::endl;
    }
}
