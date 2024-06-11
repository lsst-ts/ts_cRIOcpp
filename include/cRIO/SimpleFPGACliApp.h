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

#ifndef __SimpleFPGACliApp_h
#define __SimpleFPGACliApp_h

#include <functional>
#include <ostream>
#include <unistd.h>

#include <cRIO/CliApp.h>
#include <cRIO/SimpleFPGA.h>

namespace LSST {
namespace cRIO {

constexpr int NEED_FPGA = 0x01;

/**
 * Class for Command Line Client applications requiring access to FPGA.
 * Provides generic hook functions, allowing customizing by providing creating
 * custom FPGA and ILCs. ILCs are stored in list, as a @glos{ILC} class address ILCs
 * on single bus.
 */
template <class fpga>
class TemplateFPGACliApp : public CliApp {
public:
    /**
     * Construct *FPGACliApp.
     *
     * @param name application name
     * @param description a short description of the application
     */
    TemplateFPGACliApp(const char* name, const char* description)
            : CliApp(name, description), _fpga(nullptr), _autoOpen(true), _timeIt(false) {
        addArgument('d', "increase debug level");
        addArgument('h', "print this help");
        addArgument('O', "don't auto open (and run) FPGA");

        addCommand("@timeit", std::bind(&TemplateFPGACliApp::timeit, this, std::placeholders::_1), "b", 0,
                   "[flag]", "Sets timing flag");
        addCommand("close", std::bind(&TemplateFPGACliApp::closeFPGA, this, std::placeholders::_1), "",
                   NEED_FPGA, NULL, "Close FPGA connection");

        addCommand("help", std::bind(&TemplateFPGACliApp::helpCommands, this, std::placeholders::_1), "s", 0,
                   "[command]", "Print commands help");
        addCommand("open", std::bind(&TemplateFPGACliApp::openFPGA, this, std::placeholders::_1), "", 0, NULL,
                   "Open FPGA");
        addCommand("verbose", std::bind(&TemplateFPGACliApp::verbose, this, std::placeholders::_1), "?", 0,
                   "<new level>", "Report/set verbosity level");
    }

    /**
     * Class destructor. Subclasses are encouraged to include all destruction
     * steps in their own destructor.
     */
    virtual ~TemplateFPGACliApp() {}

    /**
     * Run the application.
     *
     * @return application return code
     */
    virtual int run(int argc, char* const argv[]) {
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

    int timeit(command_vec cmds) {
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

    int setIlcTimeout(command_vec cmds);

    int closeFPGA(command_vec cmds) {
        _fpga->close();
        if (_fpga_singleton == false) {
            delete _fpga;
        }
        _fpga = nullptr;
        return 0;
    }

    int openFPGA(command_vec cmds) {
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
        _fpga_singleton = false;
        _fpga = newFPGA(dir, _fpga_singleton);
        _fpga->initialize();
        _fpga->open();
        return 0;
    }

    int verbose(command_vec cmds) {
        switch (cmds.size()) {
            case 1:
                setDebugLevel(std::stoi(cmds[0]));
            case 0:
                std::cout << "Debug level: " << getDebugLevel() << std::endl;
                break;
        }
        return 0;
    }

protected:
    void processArg(int opt, char* optarg) override {
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

    int processCommand(Command* cmd, const command_vec& args) override {
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

    /**
     * Creates new FPGA class. Pure virtual, must be overloaded.
     *
     * @param dir directory with
     * @param fpga_singleton set to true if the returned pointer points to a singleton
     *
     * @return new FPGA class
     */
    virtual fpga* newFPGA(const char* dir, bool& fpga_singleton) = 0;

    fpga* getFPGA() { return _fpga; }

private:
    fpga* _fpga;

    bool _autoOpen;
    bool _timeIt;
    bool _fpga_singleton;
};

/**
 * Class for Command Line Client applications requiring access to FPGA.
 * Provides generic hook functions, allowing customizing by providing creating
 * custom FPGA and ILCs. ILCs are stored in list, as a @glos{ILC} class address ILCs
 * on single bus.
 */
typedef TemplateFPGACliApp<SimpleFPGA> SimpleFPGACliApp;

}  // namespace cRIO
}  // namespace LSST

#endif  //! __SimpleFPGACliApp_h
