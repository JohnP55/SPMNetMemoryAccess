#pragma once
#include "common.h"

#include <EASTL/map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
using eastl::string;

namespace mod {

typedef u32 (*CommandCb)(eastl::vector<const char*> &args, u8* response, size_t responseSize);

class Command {
friend class CommandManager;
public:
    Command(const char* name, const char* helpMsg, s32 argc, CommandCb cb);
    u32 execute(eastl::vector<const char*> &args, u8* response, size_t responseSize) const;
    const char* getName() const;
    const char* getHelpMsg() const;
    s32 getArgc() const;
private:
    const char* name;
    const char* helpMsg;
    s32 argc;
    CommandCb cb;
};

class CommandManager {
public:
    ~CommandManager();
    static CommandManager* CreateInstance();
    static CommandManager* Instance();
    const Command* findCommand(const char* name);
    bool addCommand(const Command* cmd);
    bool removeCommand(const char* name);
    u32 parseAndExecute(const char* str, u8* response, size_t responseSize);

    eastl::vector<const Command*> commands;
private:
    CommandManager();
    static CommandManager* s_instance;
};

extern "C" {
    void initCommands();
}

}