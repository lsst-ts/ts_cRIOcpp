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

#ifndef __Application_h
#define __Application_h

#include <string>
#include <vector>
#include <list>

#include <spdlog/spdlog.h>

namespace LSST {
namespace cRIO {

typedef std::vector<std::string> command_vec;

struct Argument {
    Argument(char _arg, const char* _help, char _modifer) {
        arg = _arg;
        help = _help;
        modifier = _modifer;
    }
    char arg;
    const char* help;
    char modifier;
};

/**
 * Core class for a command line (and interactive) application. Provides
 * functions for parsing command line arguments, using readline and history in
 * the interactive prompt, and providing help for commands.
 *
 * The class shall be value initialized. See the following example code.
 *
 * @code
#include <cRIO/Application.hpp>
#include <iostream>

AClass : public cRIO::Application {
public:
    AClass(const char * description) : Application(description), counter(0) {}
    float counter;

protected:
    void printUsage() override;
    void processArg(int opt, char * optarg) override;
};

void AClass::printUsage() {
    std::cout << "A simple app. Accept -h for help. Pass -i to start interactive mode." << std::endl;
    Application::printUsage();
}

void AClass::processArg(int opt, char * optarg) {
    switch (opt)
    {
        case 'a':
            counter += std::stof(optarg);
            break;
        case 'h':
            printAppHelp();
            break;
        default:
            std::cerr << "Unknow command: " << dynamic_cast<char>(opt) << std::endl;
            exit(EXIT_FAILURE);
    }
}

AClass cli("description");

int main(int argc, char * argv[])
{
    cli.processArgs(argc, argv);
    std::cout << "Counter is " << cli.counter << std::endl;
    return 0;
}
 * @endcode
 */
class Application {
public:
    /**
     * Construct CliApp.
     *
     * @param _description a short description of the application
     */
    Application(const char* description = NULL) : _description(description), _debugLevel(0) {}

    /**
     * Class destructor. Subclasses are encouraged to include all destruction
     * steps in their own destructor.
     */
    virtual ~Application();

    /**
     * Sets CLI description. Description is printed in help text.
     *
     * @param description new description
     */
    void setDescription(const char* description) { _description = description; }

    /**
     * Add argument. Should be called before call to processArgs.
     *
     * @param arg command line argument
     * @param help help string
     * @param modifer optarg modifier - : for required parameter, ? for
     * optional parameter
     */
    void addArgument(const char arg, const char* help, const char modifer = 0);

    /**
     * Initialize the class, parses arguments. Argument parsing stops after the
     * first non-argument. This allows for calls as:
     *
     * app -v -c config.txt command -1.0 -3.141592
     *
     * to pass negative numbers.
     *
     * @param argc argument count (from main method)
     * @param argv argument values (from main method)
     *
     * @return vector with command passed on the command line (after arguments)
     *
     * @see Command
     */
    command_vec processArgs(int argc, char* const argv[]);

    /**
     * Prints application help.
     */
    virtual void printAppHelp();

    std::string getName() { return _name; }
    void setName(std::string name) { _name = name; }

    typedef enum { STDOUT = 0x01, DAILY = 0x02, SYSLOG = 0x04, SAL = 0x10 } Sinks;
    int enabledSinks;

    int getDebugLevel() { return _debugLevel; }
    void setDebugLevel(int newLevel);

protected:
    /**
     * Prints application usage.
     */
    virtual void printUsage();

    /**
     * Prints preamble for commands help. Can include tips which commands to
     * use,..
     */
    virtual void printGenericHelp(){};

    /**
     * Process option from command line parsing.
     *
     * @param opt option
     * @param optarg option argument, can be null
     *
     * @return -1 on error, 0 on success (shall continue)
     */
    virtual void processArg(int opt, char* optarg) = 0;

    void addSink(spdlog::sink_ptr sink) { _sinks.push_back(sink); }
    virtual void setSinks();

    spdlog::level::level_enum getSpdLogLogLevel();
    void incDebugLevel() { _debugLevel++; }

private:
    const char* _description;
    std::list<Argument> _arguments;
    std::string _name;
    std::vector<spdlog::sink_ptr> _sinks;

    int _debugLevel;
};

}  // namespace cRIO
}  // namespace LSST

#endif  //! __Application_h
