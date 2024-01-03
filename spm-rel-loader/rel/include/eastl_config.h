#pragma once

#include <common.h>
#include <spm/system.h>

// Set platform as linux powerpc
#define __powerpc__
#define EA_PLATFORM_LINUX

// Use spm assert
// TODO: stub when EA assert flags are off
#define EASTL_ASSERT(expression) assert(expression, "EASTL_ASSERT")
#define EASTL_ASSERT_MSG(expression, message) assertf(expression, "EASTL_ASSERT_MSG: %s", message)
#define EASTL_FAIL_MSG(message) assertf(0, "EASTL_FAIL_MSG: %s", message)