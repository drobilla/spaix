/*
  Copyright 2013-2020 David Robillard <d@drobilla.net>

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TEST_OPTIONS_HPP
#define TEST_OPTIONS_HPP

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

namespace spaix {
namespace test {

struct Option
{
  const char* help;
  const char* value_name;
  const char* default_value;
};

using Options   = std::map<std::string, Option>;
using Arguments = std::map<std::string, std::string>;

inline void
print_usage(const std::string& prog, const Options& options)
{
  std::cerr << "Usage: " << prog << " [OPTION]...\n\n";
  size_t name_width  = 0;
  size_t value_width = 0;
  for (const auto& o : options) {
    const char* value_name = o.second.value_name;

    name_width  = std::max(name_width, o.first.length());
    value_width = std::max(value_width, value_name ? strlen(value_name) : 0);
  }

  for (const auto& o : options) {
    std::cerr << "  --";
    std::cerr.width(static_cast<std::streamsize>(name_width));
    std::cerr << std::left << o.first << " ";
    std::cerr.width(static_cast<std::streamsize>(value_width + 2));
    std::cerr << (o.second.value_name ? o.second.value_name : "")
              << o.second.help << "\n";
  }
}

inline Arguments
parse_options(const Options& opts, int argc, char** argv)
{
  std::map<std::string, std::string> args;

  for (int i = 1; i < argc; ++i) {
    const std::string arg  = argv[i];
    const std::string name = arg.substr(2);
    if (arg.substr(0, 2) != "--" || opts.find(name) == opts.end()) {
      throw std::runtime_error("Unknown argument '" + arg + "'");
    }

    const auto& opt = opts.at(arg.substr(2));
    if (opt.value_name) {
      args.emplace(name, argv[++i]);
    }
  }

  for (const auto& opt : opts) {
    if (args.find(opt.first) == args.end()) {
      args.emplace(opt.first, opt.second.default_value);
    }
  }

  return args;
}

} // namespace test
} // namespace spaix

#endif // TEST_OPTIONS_HPP
