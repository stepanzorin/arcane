#include "pipeline_functions.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string_view>

namespace sm::arcane::common::shaders {

std::pair<vertex_data_t, fragment_data_t> read_spirv_files(const std::filesystem::path &file_name) {
    static const auto spirv_dir_path = std::filesystem::path{SM_ARCANE_SPIRV_DIR_PATH};
    static constexpr auto vert_extension = std::string_view{".vert.spv"};
    static constexpr auto frag_extension = std::string_view{".frag.spv"};

    const auto vert_spirv_src_file_path{spirv_dir_path / (file_name.string() + vert_extension.data())};
    const auto frag_spirv_src_file_path{spirv_dir_path / (file_name.string() + frag_extension.data())};

    static const auto read_file_to_vector = [](const std::filesystem::path &path) {
        auto file = std::ifstream{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error{"Failed to open SPIR-V file!"};
        }

        const auto file_size = file.tellg();
        auto buffer = shader_data_t(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);
        file.close();

        return buffer;
    };

    const auto vert_data = read_file_to_vector(vert_spirv_src_file_path);
    const auto frag_data = read_file_to_vector(frag_spirv_src_file_path);

    return {vert_data, frag_data};
}

vk::raii::ShaderModule create_shader_module(const vk::raii::Device &device, const std::vector<char> &code) noexcept {
    const auto *code_raw_ptr = reinterpret_cast<const std::uint32_t *>(code.data());
    const auto create_info = vk::ShaderModuleCreateInfo{{}, code.size(), code_raw_ptr};
    return {device, create_info};
}

} // namespace sm::arcane::common::shaders