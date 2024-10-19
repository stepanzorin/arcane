#include "pretty_json.hpp"

#include <fstream>

#include <boost/json/value.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/kind.hpp>

namespace sm::arcane::util {

namespace json = boost::json;

void write_pretty_json(std::fstream &to_file, const json::value &from_desc, std::string *indent_ptr) {
    auto indent = std::string{};
    if (!indent_ptr) {
        indent_ptr = &indent;
    }

    switch (from_desc.kind()) {
        case json::kind::object: {
            to_file << "{\n";
            indent_ptr->append(2, ' ');
            const auto &obj = from_desc.get_object();
            if (!obj.empty()) {
                auto it = obj.begin();
                while (true) {
                    to_file << *indent_ptr << json::serialize(it->key()) << ": ";
                    write_pretty_json(to_file, it->value(), indent_ptr);
                    if (++it == obj.end()) {
                        break;
                    }
                    to_file << ",\n";
                }
            }
            to_file << "\n";
            indent_ptr->resize(indent_ptr->size() - 2);
            to_file << *indent_ptr << "}";
        } break;

        case json::kind::array: {
            to_file << "[";
            const auto &arr = from_desc.get_array();
            for (auto it = arr.begin(); it != arr.end(); ++it) {
                if (it != arr.begin()) {
                    to_file << ", ";
                }
                write_pretty_json(to_file, *it, indent_ptr);
            }
            to_file << "]";
            break;
        }

        case json::kind::string: {
            to_file << json::serialize(from_desc.get_string());
            break;
        }

        case json::kind::uint64:
        case json::kind::int64:
        case json::kind::double_: to_file << from_desc; break;

        case json::kind::bool_: {
            if (from_desc.get_bool()) {
                to_file << "true";
            } else {
                to_file << "false";
            }
        } break;

        case json::kind::null: to_file << "null"; break;
    }

    if (indent_ptr->empty()) {
        to_file << "\n";
    }
}

} // namespace sm::arcane::util