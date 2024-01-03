#include "commands.h"
#include <spm/evt_mario.h>
#include <spm/evt_msg.h>
#include <spm/evtmgr.h>
#include <spm/system.h>
#include <msl/stdio.h>

namespace mod {

inline bool isWithinMem1Range(s32 ptr) {
    return (ptr >= 0x80000000 && ptr <= 0x817fffff);
}

COMMAND(read, "Reads memory from the console. (read address n)", 2, {
    u32 ptr = strtoul(args[0], NULL, 16);
    u32 size = strtoul(args[1], NULL, 10);
    if (size > responseSize) {
        msl::stdio::snprintf((char*)response, responseSize, "Error: Requested more bytes (%d) than the allocated buffer's size (%d).\n", size, responseSize);
        return (u32)msl::string::strlen((char*)response);
    }
    if (ptr < 0x80000000 || ptr + size > 0x817fffff) {
        msl::stdio::snprintf((char*)response, responseSize, "Error: Address (%x) or end of buffer (%x) outside of MEM1's range (0x80000000 - 0x817fffff).\n", ptr, ptr + size);
        return (u32)msl::string::strlen((char*)response);
    }
    msl::string::memcpy(response, (void*)ptr, size);
    return size;
})

COMMAND(write, "Writes memory to the console. (write address base64_encoded_bytearray decoded_size)", 3, {
    u32 dest = strtoul(args[0], NULL, 16);
    const char* b64data = args[1];
    s32 decodedLen = strtoul(args[2], NULL, 10);
    
    if (!isValidBase64(b64data, decodedLen)) {
        msl::stdio::snprintf((char*)response, responseSize, "Error: Invalid Base64 data.\n");
        return (u32)msl::string::strlen((char*)response);
    }
    if (!isWithinMem1Range(dest) || !isWithinMem1Range(dest+decodedLen)) {
        msl::stdio::snprintf((char*)response, responseSize, "Error: Address (%x) or end of buffer (%x) outside of MEM1's range (0x80000000 - 0x817fffff).\n", dest, dest + decodedLen);
        return (u32)msl::string::strlen((char*)response);
    }
    char* decodedData = new char[decodedLen];
    Base64decode(decodedData, b64data);
    msl::string::memcpy((void*)dest, decodedData, decodedLen);
    delete[] decodedData;

    msl::string::memcpy((void*)response, &decodedLen, sizeof(s32));
    return (u32)sizeof(s32);
})

EVT_BEGIN(msgbox_cmd)
    USER_FUNC(spm::evt_mario::evt_mario_key_off, 1)
    USER_FUNC(spm::evt_msg::evt_msg_print, 1, LW(0), 0, 0)
    USER_FUNC(spm::evt_msg::evt_msg_continue)
    USER_FUNC(evt_post_msgbox, LW(0))
    USER_FUNC(spm::evt_mario::evt_mario_key_on)
    RETURN()
EVT_END()
EVT_BEGIN(fwd_msgbox_cmd)
    RUN_EVT(msgbox_cmd)
    RETURN()
EVT_END()

COMMAND(msgbox, "Displays a message box on the screen. (msgbox base64_encoded_string size)", 2, {
    const char* b64data = args[0];
    s32 msgboxTextLen = strtoul(args[1], NULL, 10);
    
    if (!isValidBase64(b64data, msgboxTextLen)) {
        msl::stdio::snprintf((char*)response, responseSize, "Error: Invalid Base64 data.\n");
        return msl::string::strlen((char*)response);
    }

    char* msgboxText = new char[msgboxTextLen + 1];
    Base64decode(msgboxText, b64data);
    msgboxText[msgboxTextLen] = '\0';
    spm::evtmgr::EvtEntry* evt = spm::evtmgr::evtEntry(fwd_msgbox_cmd, 0, 0);
    evt->lw[0] = msgboxText;

    msl::string::memcpy((void*)response, &msgboxTextLen, sizeof(s32));
    return sizeof(s32);
})

EVT_DEFINE_USER_FUNC(evt_post_msgbox) {
    char* textPtr = reinterpret_cast<char*>(spm::evtmgr_cmd::evtGetValue(evt, evt->pCurData[0]));
    delete[] textPtr;
    return EVT_RET_CONTINUE;
}

EVT_DEFINE_USER_FUNC(evt_deref) {
    s32 addr = spm::evtmgr_cmd::evtGetValue(evt, evt->pCurData[0]);
    assert(isWithinMem1Range(addr));
    s32* ptr = reinterpret_cast<s32*>(addr);
    spm::evtmgr_cmd::evtSetValue(evt, evt->pCurData[1], *ptr);
    return EVT_RET_CONTINUE;
}

}