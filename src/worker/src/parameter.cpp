/**
 * @brief
 * @author Yuki Koizumi
 */
#include "parameter.hpp"
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>

Parameter &Parameter::instance() {
  static Parameter object;
  return object;
}

Parameter::Parameter()
    : m_cd("/image/location:30123"),
      m_worker_name("/worker01"),
      m_location_name("30123"),
      m_time_name(),
      m_dummy_file(),
      m_is_dummy_mode(false),
      m_is_emulation_mode(false)
{}

void Parameter::parse(int argc, char **argv) {
  boost::program_options::options_description cmdline_opt("Command line options");

  try {
    cmdline_opt.add_options()
        ("help,h", "Show this help message")
        ("cd,c", boost::program_options::value<std::string>(),
         "Content descriptor for pub/sub communication")
        ("worker,w", boost::program_options::value<std::string>(),
         "Name of this worker used for pseudo RICE communication")
        ("location,l", boost::program_options::value<std::string>(),
         "Location name specified by Z-ordering")
        ("time,t", boost::program_options::value<std::string>(),
         "Time name")
        ("dummy,d", boost::program_options::value<std::string>(),
         "Run in dummy mode with specified dummy file")
        ("emulation,e", "Run in emulation mode");

    boost::program_options::options_description opt("Options");
    opt.add(cmdline_opt);

    boost::program_options::variables_map parameters;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt), parameters);
    boost::program_options::notify(parameters);

    if(parameters.count("help")) {
      std::cerr << cmdline_opt << std::endl;
      exit(0);
    }

    if(parameters.count("cd")) {
      m_cd = parameters["cd"].as<std::string>();
    }
    if(parameters.count("worker")) {
      m_worker_name = parameters["worker"].as<std::string>();
    }
    if(parameters.count("location")) {
      m_location_name = parameters["location"].as<std::string>();
    }
    if(parameters.count("time")) {
      m_time_name = parameters["time"].as<std::string>();
    }
    if(parameters.count("dummy")) {
      m_dummy_file = parameters["dummy"].as<std::string>();
      m_is_dummy_mode = true;
    }
    if(parameters.count("emulation")) {
      m_is_emulation_mode = true;
    }

  } catch(std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    exit(1);
  } catch(...) {
    std::cerr << "Catch unknown exception" << std::endl;
    exit(1);
  }

  return;
}

void Parameter::print(std::ostream &os) const {
  boost::format console_format("%1%:%|38t|%2%");
  // boost::format console_format("%1%:\t%2%");
  os << "Parameters" << std::endl;
  os << console_format % "Content descriptor" % m_cd << std::endl;
  os << console_format % "Worker name" % m_worker_name << std::endl;
  os << console_format % "Location name" % m_location_name << std::endl;
  os << console_format % "Time name" % m_time_name << std::endl;
  os << console_format % "Dummy mode" % (Parameter::instance().is_dummy_mode() ? "On" : "Off") << std::endl;
  os << console_format % "Emulation mode" % (Parameter::instance().is_emulation_mode() ? "On" : "Off") << std::endl;
  os << std::endl;

  return;
}

std::ostream &operator<<(std::ostream &os, const Parameter &obj) {
  obj.print(os);
  return os;
}
