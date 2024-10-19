// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <iosfwd>
#include <string>

#include <boost/json/fwd.hpp>

namespace sm::arcane::util {

void write_pretty_json(std::fstream &to_file, const boost::json::value &from_desc, std::string *indent_ptr = nullptr);

} // namespace sm::arcane::util