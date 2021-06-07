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

#ifndef __SAL_Command_
#define __SAL_Command_

#include <cRIO/Command.h>

namespace LSST {
namespace cRIO {
namespace SAL {
constexpr int ACK_INPROGRESS = 301;
constexpr int ACK_COMPLETE = 303;
constexpr int ACK_FAILED = -302;
}  // namespace SAL
}  // namespace cRIO
}  // namespace LSST

#define SAL_COMMAND_CLASS(prefix, sal, command)                                                              \
    class SAL_##command : public cRIO::Command {                                                             \
    public:                                                                                                  \
        SAL_##command(int32_t commandId, prefix##_command_##command##C* data) : params(*data) {              \
            _commandId = commandId;                                                                          \
        }                                                                                                    \
                                                                                                             \
        void execute() override;                                                                             \
        void ackInProgress() override {                                                                      \
            sal->ackCommand_##command(_commandId, LSST::cRIO::SAL::ACK_INPROGRESS, 0, (char*)"In-Progress"); \
        }                                                                                                    \
                                                                                                             \
        void ackComplete() override {                                                                        \
            sal->ackCommand_##command(_commandId, LSST::cRIO::SAL::ACK_COMPLETE, 0, (char*)"Complete");      \
        }                                                                                                    \
                                                                                                             \
        void ackFailed(std::string reason) override {                                                        \
            sal->ackCommand_##command(_commandId, LSST::cRIO::SAL::ACK_FAILED, 0,                            \
                                      (char*)("Failed:" + reason).c_str());                                  \
        }                                                                                                    \
                                                                                                             \
    private:                                                                                                 \
        int _commandId;                                                                                      \
        prefix##_command_##command##C params;                                                                \
    }

#define SAL_COMMAND_CLASS_validate(prefix, sal, command)                                                     \
    class SAL_##command : public cRIO::Command {                                                             \
    public:                                                                                                  \
        SAL_##command(int32_t commandId, prefix##_command_##command##C* data) : params(*data) {              \
            _commandId = commandId;                                                                          \
        }                                                                                                    \
                                                                                                             \
        bool validate() override;                                                                            \
        void execute() override;                                                                             \
        void ackInProgress() override {                                                                      \
            sal->ackCommand_##command(_commandId, LSST::cRIO::SAL::ACK_INPROGRESS, 0, (char*)"In-Progress"); \
        }                                                                                                    \
                                                                                                             \
        void ackComplete() override {                                                                        \
            sal->ackCommand_##command(_commandId, LSST::cRIO::SAL::ACK_COMPLETE, 0, (char*)"Complete");      \
        }                                                                                                    \
                                                                                                             \
        void ackFailed(std::string reason) override {                                                        \
            sal->ackCommand_##command(_commandId, LSST::cRIO::SAL::ACK_FAILED, 0,                            \
                                      (char*)("Failed:" + reason).c_str());                                  \
        }                                                                                                    \
                                                                                                             \
    private:                                                                                                 \
        int _commandId;                                                                                      \
        prefix##_command_##command##C params;                                                                \
    }

#endif  //! __SAL_Command_
