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

#include <functional>
#include <ostream>

#include <cRIO/CliApp.h>
#include <cRIO/FPGA.h>
#include <cRIO/PrintILC.h>

namespace LSST {
namespace cRIO {

constexpr int NEED_FPGA = 0x01;

typedef std::pair<std::shared_ptr<PrintILC>, uint8_t> ILCUnit;
typedef std::list<ILCUnit> ILCUnits;

/**
 * Prints ILCUnit bus address and @glos{ILC} address.
 */
std::ostream& operator<<(std::ostream& stream, ILCUnit const& u);

/**
 * Class for Command Line Client applications requiring access to FPGA.
 * Provides generic hook functions, allowing customizing by providing creating
 * custom FPGA and ILCs. ILCs are stored in list, as a @glos{ILC} class address ILCs
 * on single bus.
 */
class FPGACliApp : public CliApp {
public:
    /**
     * Construct FPGACliApp.
     *
     * @param name application name
     * @param description a short description of the application
     */
    FPGACliApp(const char* name, const char* description);

    /**
     * Class destructor. Subclasses are encouraged to include all destruction
     * steps in their own destructor.
     */
    virtual ~FPGACliApp();

    /**
     * Run the application.
     *
     * @return application return code
     */
    virtual int run(int argc, char* const argv[]);

    int timeit(command_vec cmds);
    int setIlcTimeout(command_vec cmds);

    int closeFPGA(command_vec cmds);
    int openFPGA(command_vec cmds);
    int programILC(command_vec cmds);
    int verbose(command_vec cmds);

protected:
    void addILCCommand(const char* command, std::function<void(ILCUnit)> action, const char* help);

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
    std::shared_ptr<PrintILC> getILC(int index) { return _ilcs[index]; }

    void addILC(std::shared_ptr<PrintILC> ilc) { _ilcs.push_back(ilc); }

    void addMPU(const char* name, std::shared_ptr<MPU> mpu) { _mpu.emplace(name, mpu); }

    void printMPU();

    std::shared_ptr<MPU> getMPU(std::string name);

    /**
     * Clears @glos{ILC} buffers. Shall be called before new instructions are added to
     * the buffer.
     */
    void clearILCs() {
        for (auto ilcp : _ilcs) {
            ilcp->clear();
        }
    }

    /**
     * Run prepared @glos{ILC} commands on @glos{ILC} busses.
     *
     * @param timeout @glos{ILC} timeout (in ms)
     */
    void runILCCommands(int32_t timeout) {
        for (auto ilcp : _ilcs) {
            if (ilcp->empty() == false) {
                _fpga->ilcCommands(*ilcp, timeout);
            }
        }
    }

    /**
     * Disable given ILC.
     *
     * @param u @glos{ILC} unit to disable.
     */
    void disableILC(ILCUnit u);

    /**
     * Enables given ILC.
     *
     * @param u @glos{ILC} unit to enable.
     */
    void enableILC(ILCUnit u);

    /**
     * Prints actually disabled ILCs.
     */
    void printDisabled();

    int32_t ilcTimeout;

private:
    FPGA* _fpga;
    std::vector<std::shared_ptr<PrintILC>> _ilcs;
    std::map<std::string, std::shared_ptr<MPU>> _mpu;

    ILCUnits _disabledILCs;

    bool _autoOpen;
    bool _timeIt;
};

}  // namespace cRIO
}  // namespace LSST

#endif  //!
