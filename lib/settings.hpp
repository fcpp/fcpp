// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

/**
 * @file settings.hpp
 * @brief Definition of default values for settings macros.
 */

#ifndef FCPP_SETTINGS_H_
#define FCPP_SETTINGS_H_


//! @brief Identifier for general systems.
#define FCPP_GENERAL  1
//! @brief Identifier for embedded systems.
#define FCPP_EMBEDDED 2


#ifndef FCPP_SETTING_ENVIRONMENT
//! @brief Setting defining the overall environment (defaults to @ref FCPP_GENERAL).
#define FCPP_SETTING_ENVIRONMENT FCPP_GENERAL
#endif


#if   FCPP_SETTING_ENVIRONMENT == FCPP_GENERAL
    #ifndef FCPP_SETTING_TRACE
    //! @brief Setting defining the size of trace hashes (64 for general systems, 32 for embedded systems).
    #define FCPP_SETTING_TRACE 64
    #endif
    #ifndef FCPP_SETTING_DEVICE
    //! @brief Setting defining the size of device identifiers (32 for general systems, 16 for embedded systems).
    #define FCPP_SETTING_DEVICE 32
    #endif
    #ifndef FCPP_SETTING_EXPORTS
    //! @brief Settings defining whether exports for self and other devices should be separated (1 for general systems, 2 for embedded systems).
    #define FCPP_SETTING_EXPORTS 1
    #endif
#elif FCPP_SETTING_ENVIRONMENT == FCPP_EMBEDDED
    #ifndef FCPP_SETTING_TRACE
    //! @brief Setting defining the size of trace hashes (64 for general systems, 32 for embedded systems).
    #define FCPP_SETTING_TRACE 32
    #endif
    #ifndef FCPP_SETTING_DEVICE
    //! @brief Setting defining the size of device identifiers (32 for general systems, 16 for embedded systems).
    #define FCPP_SETTING_DEVICE 16
    #endif
    #ifndef FCPP_SETTING_EXPORTS
    //! @brief Settings defining whether exports for self and other devices should be separated (1 for general systems, 2 for embedded systems).
    #define FCPP_SETTING_EXPORTS 2
    #endif
#else
    static_assert(false, "invalid value for FCPP_SETTING_ENVIRONMENT");
#endif


#endif // FCPP_SETTINGS_H_
