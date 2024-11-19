#include "viewpoint.hpp"

#include <fstream>
#include <iterator>
#include <string>

#include <boost/json.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>

#include "util/filesystem_helpers.hpp"
#include "util/pretty_json.hpp"

namespace sm::arcane::scene {

namespace json = boost::json;

namespace {

const auto g_viewpoint_file_path = util::application_directory_path() / "viewpoint.json";

} // namespace

viewpoint_s load_viewpoint_from_json() {
    auto file = std::ifstream{g_viewpoint_file_path};

    const auto content = std::string{(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
    auto json_val = json::parse(content);
    auto obj = json_val.as_object();

    auto position = obj["position"].as_array();
    auto orientation = obj["orientation"].as_array();

    const auto t = position[0].as_double();
    return {.position = {position[0].as_double(), position[1].as_double(), position[2].as_double()},
            .orientation = {static_cast<float>(orientation[0].as_double()),
                            static_cast<float>(orientation[1].as_double()),
                            static_cast<float>(orientation[2].as_double()),
                            static_cast<float>(orientation[3].as_double())}};
}
void save_viewpoint_to_json(const viewpoint_s &viewpoint) {
    auto file = std::ofstream{g_viewpoint_file_path};

    auto obj = boost::json::object{};
    obj["position"] = json::array{viewpoint.position.x, viewpoint.position.y, viewpoint.position.z};
    obj["orientation"] = json::array{viewpoint.orientation.w,
                                     viewpoint.orientation.x,
                                     viewpoint.orientation.y,
                                     viewpoint.orientation.z};

    util::write_pretty_json(file, obj);
}

} // namespace sm::arcane::scene