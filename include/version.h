/**
 * @file version.h
 * @brief UVAPI Version Information
 * 
 * This file contains version information for the UVAPI framework.
 * Version follows Semantic Versioning 2.0.0 (semver.org)
 * Format: MAJOR.MINOR.PATCH
 * 
 * MAJOR: Incompatible API changes
 * MINOR: Backwards-compatible functionality additions
 * PATCH: Backwards-compatible bug fixes
 */

#ifndef UVAPI_VERSION_H
#define UVAPI_VERSION_H

#define UVAPI_VERSION_MAJOR 1
#define UVAPI_VERSION_MINOR 1
#define UVAPI_VERSION_PATCH 1

#define UVAPI_VERSION_STRING "1.1.1"
#define UVAPI_VERSION_NUMBER 10101

// Version helper macros
#define UVAPI_VERSION_AT_LEAST(major, minor, patch) \
    ((UVAPI_VERSION_MAJOR > (major)) || \
     (UVAPI_VERSION_MAJOR == (major) && UVAPI_VERSION_MINOR > (minor)) || \
     (UVAPI_VERSION_MAJOR == (major) && UVAPI_VERSION_MINOR == (minor) && UVAPI_VERSION_PATCH >= (patch)))

#define UVAPI_VERSION_EXACT(major, minor, patch) \
    (UVAPI_VERSION_MAJOR == (major) && \
     UVAPI_VERSION_MINOR == (minor) && \
     UVAPI_VERSION_PATCH == (patch))

#endif // UVAPI_VERSION_H