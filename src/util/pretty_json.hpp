#pragma once

#include <iosfwd>
#include <string>

#include <boost/json/fwd.hpp>

namespace sm::arcane::util {

void write_pretty_json(std::fstream &to_file, boost::json::value const &from_desc, std::string *indent_ptr = nullptr);

} // namespace sm::arcane::util