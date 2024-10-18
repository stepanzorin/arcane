from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

required_conan_version = "^1.65.0"


class ArcaneConan(ConanFile):
    name = "arcane"
    version = "1.0.1"
    description = "The game engine based on Vulkan API"
    url = "https://github.com/stepanzorin/arcane"
    author = "Stepan Zorin <stz.hom@gmail.com>"
    homepage = url

    settings = "arch", "build_type", "compiler", "os"
    generators = "CMakeDeps", "CMakeToolchain"

    def configure(self):
        # Since 1.84.0 the Boost library requires Boost.Cobalt
        self.options["boost"].without_cobalt = True

    def requirements(self):  # [ FORWARD SORT BY PACKAGE NAME ]
        # third-party packages
        self.requires("boost/1.85.0")
        self.requires("fmt/10.2.1")
        self.requires("glfw/3.4")
        self.requires("glm/cci.20230113")
        self.requires("gtest/1.14.0")
        self.requires("range-v3/cci.20240905")
        self.requires("spdlog/1.14.0")
        self.requires("stb/cci.20240531")   # TODO: will find a better package for working with images
        self.requires("tinyobjloader/2.0.0-rc10")
        self.requires("vulkan-headers/1.3.290.0")
        self.requires("vulkan-loader/cci.20231120", override=True)
        self.requires("vulkan-memory-allocator/cci.20231120")
        self.requires("vulkan-validationlayers/1.3.239.0", override=True)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def layout(self):
        cmake_layout(self)
