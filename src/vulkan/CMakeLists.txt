target_sources(
    arcane
    PRIVATE # cmake-format: sort
            config.cpp
            config.hpp
            device.cpp
            device.hpp
            device_memory.cpp
            device_memory.hpp
            image_barriers.hpp
            instance.cpp
            instance.hpp
            swapchain.cpp
            swapchain.hpp
            vma_wrapper.cpp
            vma_wrapper.hpp)
