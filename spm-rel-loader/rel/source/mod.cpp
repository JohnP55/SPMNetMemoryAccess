#include "mod.h"
#include "commandmanager.h"
#include "patch.h"
#include "netmemoryaccess.h"
#include "network.h"
#include "mod_ui_base/colours.h"
#include "mod_ui_base/window.h"

#include <spm/fontmgr.h>
#include <spm/seqdrv.h>
#include <spm/seqdef.h>
#include <spm/spmario.h>
#include <wii/os/OSError.h>
#include <msl/stdio.h>

namespace mod {

/*
    Title Screen Custom Text
    Prints "SPM Rel Loader" at the top of the title screen
*/

static spm::seqdef::SeqFunc *seq_titleMainReal;
static void seq_titleMainOverride(spm::seqdrv::SeqWork *wp)
{
    wii::gx::GXColor _colour = colours::green;
    f32 scale = 0.8f;
    char msg[128];
    u32 ip = Mynet_gethostip();
    msl::stdio::snprintf(msg, 128, "%d.%d.%d.%d\n", ip >> 24 & 0xff, ip >> 16 & 0xff, ip >> 8 & 0xff, ip & 0xff);
    //wii::stdio::snprintf(msg, 128, "%d\n", net_init_ret);
    spm::fontmgr::FontDrawStart();
    spm::fontmgr::FontDrawEdge();
    spm::fontmgr::FontDrawColor(&_colour);
    spm::fontmgr::FontDrawScale(scale);
    spm::fontmgr::FontDrawNoiseOff();
    spm::fontmgr::FontDrawRainbowColorOff();
    f32 x = -((spm::fontmgr::FontGetMessageWidth(msg) * scale) / 2);
    spm::fontmgr::FontDrawString(x, 200.0f, msg);
    seq_titleMainReal(wp);
}
static void titleScreenCustomTextPatch()
{
    seq_titleMainReal = spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main;
    spm::seqdef::seq_data[spm::seqdrv::SEQ_TITLE].main = &seq_titleMainOverride;
}

/*
    spmarioMain hook
*/

static void (*spmarioMainReal)();

void spmarioMainPatch()
{
    spmarioMainReal = patch::hookFunction(spm::spmario::spmarioMain,
        []()
        {
            Window::windowMain();
            spmarioMainReal();
        }
    );
}


/*
    General mod functions
*/

void main()
{
    wii::os::OSReport("SPM Rel Loader: the mod has ran!\n");

    spmarioMainPatch();

    wii::os::OSReport("Initializing NetMemoryAccess\n");
    NetMemoryAccess::init();
    wii::os::OSReport("Initialized.\n");

    titleScreenCustomTextPatch();
}

}
