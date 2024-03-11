/*
 * ILC Bus List handling communication (receiving and
 *
 * Developed for the Vera C. Rubin Observatory Telescope & Site Software Systems.
 * This product includes software developed by the Vera C.Rubin Observatory Project
 * (https://www.lsst.org). See the COPYRIGHT file at the top-level directory of
 * this distribution for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <spdlog/spdlog.h>

#include <ILC/ILCBusList.h>

using namespace ILC;

ILCBusList::ILCBusList(uint8_t bus) : _bus(bus) {
    addResponse(
            ILC_CMD::SERVER_ID,
            [this](Modbus::Parser parser) {
                uint8_t fnLen = parser.read<uint8_t>();
                if (fnLen < 12) {
                    throw std::runtime_error(fmt::format(
                            "invalid ILC function 17 response length - expect at least 12, got {}", fnLen));
                }
                fnLen -= 12;

                uint64_t uniqueID = parser.readU48();
                uint8_t ilcAppType = parser.read<uint8_t>();
                uint8_t networkNodeType = parser.read<uint8_t>();
                uint8_t ilcSelectedOptions = parser.read<uint8_t>();
                uint8_t networkNodeOptions = parser.read<uint8_t>();
                uint8_t majorRev = parser.read<uint8_t>();
                uint8_t minorRev = parser.read<uint8_t>();
                std::string fwName = parser.readString(fnLen);
                parser.checkCRC();
                processServerID(parser.address(), uniqueID, ilcAppType, networkNodeType, ilcSelectedOptions,
                                networkNodeOptions, majorRev, minorRev, fwName);
            },
            145);

    addResponse(
            ILC_CMD::SERVER_STATUS,
            [this](Modbus::Parser parser) {
                uint8_t mode = parser.read<uint8_t>();
                uint16_t status = parser.read<uint16_t>();
                uint16_t faults = parser.read<uint16_t>();
                parser.checkCRC();
                _lastMode[parser.address()] = mode;
                processServerStatus(parser.address(), mode, status, faults);
            },
            146);

    addResponse(
            ILC_CMD::CHANGE_MODE,
            [this](Modbus::Parser parser) {
                uint16_t mode = parser.read<uint16_t>();
                parser.checkCRC();
                _lastMode[parser.address()] = mode;
                processChangeILCMode(parser.address(), mode);
            },
            193);

    addResponse(
            ILC_CMD::SET_TEMP_ADDRESS,
            [this](Modbus::Parser parser) {
                uint8_t newAddress = parser.read<uint8_t>();
                parser.checkCRC();
                processSetTempILCAddress(parser.address(), newAddress);
            },
            200);

    addResponse(
            ILC_CMD::RESET_SERVER,
            [this](Modbus::Parser parser) {
                parser.checkCRC();
                processResetServer(parser.address());
            },
            235);
}

ILCBusList::~ILCBusList() {}

void ILCBusList::broadcastFunction(uint8_t address, uint8_t func, uint32_t delay, uint8_t counter,
                                   std::vector<uint8_t> data) {
    callFunction(address, func, delay, counter, data);
}

uint8_t ILCBusList::nextBroadcastCounter() {
    _broadcastCounter++;
    if (_broadcastCounter > 15) {
        _broadcastCounter = 0;
    }
    return _broadcastCounter;
}

const char *ILCBusList::getModeStr(uint8_t mode) {
    switch (mode) {
        case Mode::Standby:
            return "Standby";
        case Mode::Disabled:
            return "Disabled";
        case Mode::Enabled:
            return "Enabled";
        case Mode::FirmwareUpdate:
            return "Firmware Updade";
        case Mode::Fault:
            return "Fault";
        default:
            return "unknow";
    }
}

std::vector<const char *> ILCBusList::getStatusString(uint16_t status) {
    std::vector<const char *> ret;

    if (status & ILCStatus::MajorFault) {
        ret.push_back("Major Fault");
    }
    if (status & ILCStatus::MinorFault) {
        ret.push_back("Minor Fault");
    }
    // 0x0003 reserved
    if (status & FaultOverride) {
        ret.push_back("Fault Override");
    }
    // remaining status is ILC specific, implemented in its *ILC subclass

    return ret;
}

std::vector<const char *> ILCBusList::getFaultString(uint16_t fault) {
    std::vector<const char *> ret;

    if (fault & ILCFault::UniqueIRC) {
        ret.push_back("Unique ID CRC error");
    }
    if (fault & ILCFault::AppType) {
        ret.push_back("App Type & Network Node Type do not match");
    }
    if (fault & ILCFault::NoILC) {
        ret.push_back("No ILC App programmed");
    }
    if (fault & ILCFault::ILCAppCRC) {
        ret.push_back("ILC App CRC error");
    }
    if (fault & ILCFault::NoTEDS) {
        ret.push_back("No TEDS found");
    }
    if (fault & ILCFault::TEDS1) {
        ret.push_back("TEDS copy 1 error");
    }
    if (fault & ILCFault::TEDS2) {
        ret.push_back("TEDS copy 2 error");
    }
    // 0x0080 reserved
    if (fault & ILCFault::WatchdogReset) {
        ret.push_back("Reset due to Watchdog Timeout");
    }
    if (fault & ILCFault::BrownOut) {
        ret.push_back("Brown Out");
    }
    if (fault & ILCFault::EventTrap) {
        ret.push_back("Event Trap");
    }
    // 0x0800 Electromechanical only
    if (fault & ILCFault::SSR) {
        ret.push_back("SSR power fail");
    }
    if (fault & ILCFault::AUX) {
        ret.push_back("Aux power fail");
    }

    return ret;
}

void ILCBusList::changeILCMode(uint8_t address, uint16_t mode) {
    uint32_t timeout = 335;
    try {
        if ((getLastMode(address) == Mode::Standby && mode == Mode::FirmwareUpdate) ||
            (getLastMode(address) == Mode::FirmwareUpdate && mode == Mode::Standby)) {
            timeout = 100000;
        }
    } catch (std::out_of_range &err) {
    }
    callFunction(address, ILC_CMD::CHANGE_MODE, timeout, mode);
}
