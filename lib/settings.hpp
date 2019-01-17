// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

/**
 * @file settings.hpp
 * @brief Definition of default values for settings macros.
 */

#ifndef FCPP_SETTINGS_H_
#define FCPP_SETTINGS_H_


//! @brief Identifier for general systems.
#define FCPP_GENERAL    111
//! @brief Identifier for embedded systems.
#define FCPP_EMBEDDED   222
//! @brief Identifier for simulated systems.
#define FCPP_SIMULATION 333
//! @brief Identifier for deployed systems.
#define FCPP_DEPLOYMENT 444

#ifndef FCPP_SETTING_SYSTEM
//! @brief Setting defining the system: @ref FCPP_EMBEDDED or @ref FCPP_GENERAL (default).
#define FCPP_SETTING_SYSTEM FCPP_GENERAL
#endif

#ifndef FCPP_SETTING_ENVIRONMENT
//! @brief Setting defining the overall environment: @ref FCPP_DEPLOYMENT or @ref FCPP_SIMULATION (default).
#define FCPP_SETTING_ENVIRONMENT FCPP_SIMULATION
#endif


#if   FCPP_SETTING_SYSTEM == FCPP_GENERAL
    #ifndef FCPP_SETTING_TRACE
    //! @brief Setting defining the size of trace hashes (64 for general systems, 32 for embedded systems).
    #define FCPP_SETTING_TRACE 64
    #endif
    #ifndef FCPP_SETTING_DEVICE
    //! @brief Setting defining the size of device identifiers (32 for general systems, 16 for embedded systems).
    #define FCPP_SETTING_DEVICE 32
    #endif
#elif FCPP_SETTING_SYSTEM == FCPP_EMBEDDED
    #ifndef FCPP_SETTING_TRACE
    //! @brief Setting defining the size of trace hashes (64 for general systems, 32 for embedded systems).
    #define FCPP_SETTING_TRACE 32
    #endif
    #ifndef FCPP_SETTING_DEVICE
    //! @brief Setting defining the size of device identifiers (32 for general systems, 16 for embedded systems).
    #define FCPP_SETTING_DEVICE 16
    #endif
#else
    static_assert(false, "invalid value for FCPP_SETTING_SYSTEM");
#endif


#if   FCPP_SETTING_ENVIRONMENT == FCPP_SIMULATION
    #ifndef FCPP_SETTING_EXPORTS
    //! @brief Settings defining whether exports for self and other devices should be separated (2, default for deployed systems) or together in a `shared_ptr` (1, default for simulated systems).
    #define FCPP_SETTING_EXPORTS 1
    #endif
#elif FCPP_SETTING_ENVIRONMENT == FCPP_DEPLOYMENT
    #ifndef FCPP_SETTING_EXPORTS
//! @brief Settings defining whether exports for self and other devices should be separated (2, default for deployed systems) or together in a `shared_ptr` (1, default for simulated systems).
    #define FCPP_SETTING_EXPORTS 2
    #endif
#else
    static_assert(false, "invalid value for FCPP_SETTING_ENVIRONMENT");
#endif


#ifndef FCPP_WARNING_TRACE
    #define FCPP_WARNING_TRACE true
#endif


#endif // FCPP_SETTINGS_H_
