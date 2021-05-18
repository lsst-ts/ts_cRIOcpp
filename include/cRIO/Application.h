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

#include <functional>
#include <fstream>
#include <string>
#include <vector>
#include <list>

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
#include <tcs/utility/CliApp.hpp>
#include <iostream>

AClass : public lbto::CliApp
{
public:
  AClass(const char * description) : CliApp(description), interactive(false) {}
  bool interactive;

protected:
  void printUsage() override;
  void processArg(int opt, const char * optarg) override;
};

void AClass::printUsage()
{
  std::cout << "A simple app. Accept -h for help. Pass -i to start interactive mode." << std::endl;
}

void AClass::processArg(int opt, const char * optarg)
{
  switch (opt)
    {
      case 'h':
        printAppHelp();
        break;
      case 'i':
        interactive = true;
        break;
      default:
        std::cerr << "Unknow command: " << dynamic_cast<char>(opt) << std::endl;
        exit(EXIT_FAILURE);
    }
}

AClass cli("description");

Command commands[] =
{
  {
    "help", [ = ](command_vec cmds) { return cli.helpCommands(cmds); }, "s", 0, "[ALL|command]",
    "Prints all command or command help."
  },
  { NULL, NULL, NULL, 0, NULL, NULL }
};

int main(int argc, char * argv[])
{
  cli.init(commands, "hi", argc, argv);
  if (cli.interactive)
    return goInteractive();
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
    Application(const char* description = NULL) : _description(description) {}

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

private:
    const char* _description;
    std::list<Argument> _arguments;
    std::string _name;
};

}  // namespace cRIO
}  // namespace LSST

#endif  //! __Application_h
