#pragma once
#include <wii/os/OSError.h>

#define LOG( message ) wii::os::OSReport message; wii::os::OSReport("\n");
#define LogError( message ) LOG(message)