#include <fmt/printf.h>

#include "app_config.hpp"

int main() {
    using namespace sm::arcane;

    constexpr app_config_s app_config;
    fmt::print("Project: {}\n"
               "Version: {}.{}.{}",
               app_config.title,
               app_config.version.major,
               app_config.version.minor,
               app_config.version.patch);

    return 0;
}