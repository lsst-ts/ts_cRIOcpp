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

#include <algorithm>
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
        : TemplateFPGACliApp<FPGA>(name, description), ilcTimeout(5000), _ilcs(), _mpu(), _disabledILCs() {
    addCommand("@ilc-timeout", std::bind(&FPGACliApp::setIlcTimeout, this, std::placeholders::_1), "i", 0,
               "[ilc timeout]", "Sets and retrieve timeout for ILC commands");

    addILCCommand("@disable", std::bind(&FPGACliApp::disableILC, this, std::placeholders::_1),
                  "Temporary disable given ILC in * commands");
    addILCCommand("@enable", std::bind(&FPGACliApp::enableILC, this, std::placeholders::_1),
                  "Re-enable given ILC in * commands");
    addILCCommand("info", [](ILCUnit u) { u.first->reportServerID(u.second); }, "Print ILC info");
    addILCCommand("status", [](ILCUnit u) { u.first->reportServerStatus(u.second); }, "Print ILC status");
    addILCCommand(
            "standby", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::Mode::Standby); },
            "Change ILC mode to standby");
    addILCCommand(
            "disable", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::Mode::Disabled); },
            "Change ILC mode to disabled");
    addILCCommand(
            "enable", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::Mode::Enabled); },
            "Change ILC mode to enabled");
    addILCCommand(
            "bootloader", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::Mode::Bootloader); },
            "Change ILC mode to booloader");
    addILCCommand(
            "clear-faults", [](ILCUnit u) { u.first->changeILCMode(u.second, ILC::Mode::ClearFaults); },
            "Clear ILC faults");
    addILCCommand("reset", [](ILCUnit u) { u.first->resetServer(u.second); }, "Reset server");

    addCommand("program-ilc", std::bind(&FPGACliApp::programILC, this, std::placeholders::_1), "FS?",
               NEED_FPGA, "<firmware hex file> <ILC...>", "Program ILC with new firmware.");
}

FPGACliApp::~FPGACliApp() {}

int FPGACliApp::setIlcTimeout(command_vec cmds) {
    if (cmds.size() == 1) {
        ilcTimeout = std::stoi(cmds[0]);
    }
    std::cout << "ILC timeout: " << ilcTimeout << std::endl;
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

                runILCCommands(ilcTimeout);

                return 0;
            },
            "s?", NEED_FPGA, "<address>...", help);
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
