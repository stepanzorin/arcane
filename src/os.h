#ifndef SM_ARCANE_OS_H
#define SM_ARCANE_OS_H

// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>


// ================================================================
// OPERATING SYSTEMS (Windows, Linux, macOS)
// ================================================================

#if defined(_WIN32) || defined(_WIN64)
    #define SM_ARCANE_OPERATING_SYSTEM_WINDOWS 1
#else
    #define SM_ARCANE_OPERATING_SYSTEM_WINDOWS 0
#endif

#if defined(__linux__) || defined(__gnu_linux__)
    #define SM_ARCANE_OPERATING_SYSTEM_LINUX 1
#else
    #define SM_ARCANE_OPERATING_SYSTEM_LINUX 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #define SM_ARCANE_OPERATING_SYSTEM_MACOS 1
#else
    #define SM_ARCANE_OPERATING_SYSTEM_MACOS 0
#endif

#if !SM_ARCANE_OPERATING_SYSTEM_WINDOWS && !SM_ARCANE_OPERATING_SYSTEM_MACOS && !SM_ARCANE_OPERATING_SYSTEM_LINUX
    #error "Platform not supported! Only Windows, macOS, and Linux are supported."
#endif


// ================================================================
// ARCHITECTURES (x86, x64)
// ================================================================

#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
    #define SM_ARCANE_ARCHITECTURE_X64 1
#else
    #define SM_ARCANE_ARCHITECTURE_X64 0
#endif

#if defined(_M_IX86) || defined(__i386__)
    #define SM_ARCANE_ARCHITECTURE_X86 1
#else
    #define SM_ARCANE_ARCHITECTURE_X86 0
#endif

#if !SM_ARCANE_ARCHITECTURE_X64 && !SM_ARCANE_ARCHITECTURE_X86
    #error "Architecture not supported! Only x86 and x64 architectures are supported."
#endif


// ================================================================
// COMPILERS
// ================================================================

#if defined(_MSC_VER)
    #define SM_ARCANE_COMPILER_MSVC 1
#else
    #define SM_ARCANE_COMPILER_MSVC 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
    #define SM_ARCANE_COMPILER_GCC 1
#else
    #define SM_ARCANE_COMPILER_GCC 0
#endif

#if defined(__clang__)
    #define SM_ARCANE_COMPILER_CLANG 1
#else
    #define SM_ARCANE_COMPILER_CLANG 0
#endif


// ================================================================
// INLINE MACROS
// ================================================================

#if defined(SM_ARCANE_COMPILER_MSVC)
    #define SM_ARCANE_FORCEINLINE __forceinline
    #define SM_ARCANE_INLINE __inline
#elif defined(SM_ARCANE_COMPILER_GCC) || defined(SM_ARCANE_COMPILER_CLANG)
    #define SM_ARCANE_FORCEINLINE __attribute__((always_inline)) inline
    #define SM_ARCANE_INLINE inline
#else
    #define SM_ARCANE_FORCEINLINE inline
    #define SM_ARCANE_INLINE inline
#endif


// ================================================================
// DISABLE WARNINGS (MSVC & GCC/Clang)
// ================================================================

#if defined(SM_ARCANE_COMPILER_MSVC)
    #define SM_ARCANE_DISABLE_WARNING_PUSH __pragma(warning(push))
    #define SM_ARCANE_DISABLE_WARNING_POP __pragma(warning(pop))
    #define SM_ARCANE_DISABLE_WARNING(warning_number) __pragma(warning(disable : warning_number))
#elif defined(SM_ARCANE_COMPILER_GCC) || defined(SM_ARCANE_COMPILER_CLANG)
    #define SM_ARCANE_DISABLE_WARNING_PUSH _Pragma("GCC diagnostic push")
    #define SM_ARCANE_DISABLE_WARNING_POP _Pragma("GCC diagnostic pop")
    #define SM_ARCANE_DISABLE_WARNING(warning_name) _Pragma(SM_ARCANE_STRINGIFY(GCC diagnostic ignored warning_name))
#else
    #define SM_ARCANE_DISABLE_WARNING_PUSH
    #define SM_ARCANE_DISABLE_WARNING_POP
    #define SM_ARCANE_DISABLE_WARNING(warning_name)
#endif


// ================================================================
// DEBUG OR RELEASE MODE
// ================================================================

#if defined(_DEBUG) || !defined(NDEBUG)
    #define SM_ARCANE_DEBUG_MODE 1
#else
    #define SM_ARCANE_RELEASE_MODE 1
#endif

#endif // SM_ARCANE_OS_H