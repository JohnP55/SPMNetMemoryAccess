#include "spm_errno.h"
#include <msl/errno.h>

s32* __errno() {
    return &msl::errno::errno;
}
