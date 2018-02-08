#include "Config.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

Config::Config(std::string const &path): path(path) { }

void Config::addOption(std::string const &name, OptionBase &option) { options[name] = &option; }

void Config::read() {
  std::ifstream f(path);
  std::string line;
  while(getline(f, line)) {
    try {
      std::stringstream ss(line);
      std::string name;
      if(!(ss >> name))
        continue;

      auto opt = options.find(name);
      if(opt != options.end()) { opt->second->read(ss); }
      else { std::cerr << "Option not found: " << name << " in file " << path << "\n"; }
    }
    catch(std::exception &e) { std::cerr << "Could not read option: " << line << " in file " << path << " (" << e.what() << ")\n"; }
  }
}

void Config::write() {
  std::ofstream of(path);
  for(auto &sp: options) {
    of << sp.first << " ";
    sp.second->write(of);
    of << "\n";
  }
}
