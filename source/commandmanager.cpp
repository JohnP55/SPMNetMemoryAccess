#include "commandmanager.h"
#include "common.h"

#include "commands.h"
#include "util.h"
#include <msl/stdio.h>
#include <msl/string.h>
#define EASTL_USER_CONFIG_HEADER <eastl_config.h>
#include <EASTL/algorithm.h>
#include <EASTL/map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <wii/os/OSError.h>
using eastl::string;

namespace mod {

void initCommands() {
    auto commandManager = CommandManager::CreateInstance();
    commandManager->addCommand(&read);
    commandManager->addCommand(&write);
    commandManager->addCommand(&msgbox);
}

CommandManager* CommandManager::s_instance = nullptr;

Command::Command(const char* name, const char* helpMsg, s32 argc, CommandCb cb) {
    this->name = name;
    this->helpMsg = helpMsg;
    this->argc = argc;
    this->cb = cb;
}

u32 Command::execute(eastl::vector<const char*> &args, u8* response, size_t responseSize) const {
    s32 realArgc = args.size();
    if (realArgc != argc && argc != ANY_ARGC) {
        msl::stdio::snprintf((char*)response, responseSize, "Error: expected %d arguments, got %d\n", argc, realArgc);
        return msl::string::strlen((char*)response);
    }
    return cb(args, response, responseSize);
}

const char* Command::getName() const {
    return name;
}
const char* Command::getHelpMsg() const {
    return helpMsg;
}
s32 Command::getArgc() const {
    return argc;
}


CommandManager::CommandManager() = default;
CommandManager::~CommandManager() = default;

CommandManager* CommandManager::CreateInstance() {
    if (s_instance == nullptr) {
        s_instance = new CommandManager;
    }
    return s_instance;
}

CommandManager* CommandManager::Instance() {
    return s_instance;
}

const Command* CommandManager::findCommand(const char* name) {
    auto cmd = eastl::find_if(commands.begin(), commands.end(), [name](const Command* cmd) { return msl::string::strcmp(cmd->getName(), name) == 0; });
    if (cmd != eastl::end(commands)) {
        return *cmd;
    }
    return nullptr;
}

bool CommandManager::addCommand(const Command* cmd) {
    if (findCommand(cmd->name) != nullptr)
        return false;
    commands.push_back(cmd);
    return true;
}

bool CommandManager::removeCommand(const char* name) {
    auto cmd = findCommand(name);
    if (cmd != nullptr) {
        commands.erase(&cmd);
        return true;
    }
    return false;
}

u32 CommandManager::parseAndExecute(const char* str, u8* response, size_t responseSize) {
    wii::os::OSReport("%s\n", str);
    eastl::vector<const char*> attrs;
    for (auto &e : split(str, ' ')) {
        attrs.push_back(e.c_str());
    }
    const Command* pCmd = findCommand(attrs[0]);
    if (pCmd == nullptr) {
        wii::os::OSReport("Can't find command %s\n", attrs[0]);
        return false;
    }
    
    attrs.erase(attrs.begin() + 0);
    return pCmd->execute(attrs, response, responseSize);
}

}