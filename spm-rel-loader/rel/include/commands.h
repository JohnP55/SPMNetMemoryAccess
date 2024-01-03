#pragma once
#include "common.h"
#include "base64.h"
#include "commandmanager.h"
#include "evt_cmd.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <msl/math.h>
#include <msl/string.h>
#include <stdlib.h>

#define ANY_ARGC -1

namespace mod {

#define COMMAND(name, description, argc, code) \
    Command name(#name, description, argc, [](eastl::vector<const char*> &args, u8* response, size_t responseSize) code);

extern Command read;
extern Command write;

extern Command msgbox;
EVT_DECLARE(msgbox_cmd)
EVT_DECLARE(fwd_msgbox_cmd)
EVT_DECLARE_USER_FUNC(evt_post_msgbox, 1)
// deref(s32* ptr, s32* outvar);
EVT_DECLARE_USER_FUNC(evt_deref, 2)

}