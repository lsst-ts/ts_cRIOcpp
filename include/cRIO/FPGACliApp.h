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

#ifndef __FPGACliApp_h
#define __FPGACliApp_h

#include <cRIO/CliApp.h>
#include <cRIO/FPGA.h>
#include <cRIO/ILC.h>

namespace LSST {
namespace cRIO {

constexpr int NEED_FPGA = 0x01;

typedef std::pair<ILC*, uint8_t> ILCUnit;
typedef std::list<ILCUnit> ILCUnits;

/**
 * Class for Command Line Client applications requiring access to FPGA.
 * Provides generic hook functions, allowing customizing by providing creating
 * custom FPGA and ILCs. ILCs are stored in list, as a ILC class address ILCs
 * on single bus.
 */
class FPGACliApp : public CliApp {
public:
    /**
     * Construct FPGACliApp.
     *
     * @param _description a short description of the application
     */
    FPGACliApp(const char* description);

    /**
     * Class destructor. Subclasses are encouraged to include all destruction
     * steps in their own destructor.
     */
    virtual ~FPGACliApp();

    int info(command_vec cmds);
    int closeFPGA(command_vec cmds);
    int openFPGA(command_vec cmds);
    int verbose(command_vec cmds);

protected:
    void processArg(int opt, char* optarg) override;
    int processCommand(Command* cmd, const command_vec& args) override;

    /**
     * Creates new FPGA class. Pure virtual, must be overloaded.
     *
     * @param dir directory with
     *
     * @return new FPGA class
     */
    virtual FPGA* newFPGA(const char* dir) = 0;
    virtual ILCUnits getILCs(command_vec arguments) = 0;

    FPGA* getFPGA() { return _fpga; }
    ILC* getILC(int index) { return _ilcs[index]; }

private:
    FPGA* _fpga;
    std::vector<ILC*> _ilcs;

    bool _autoOpen;

    void _clearILCs() {
        for (auto ilcp : _ilcs) {
            ilcp->clear();
        }
    }
};

}  // namespace cRIO
}  // namespace LSST

#endif  //!
