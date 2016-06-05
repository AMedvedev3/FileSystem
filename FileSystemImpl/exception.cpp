#include "exception.h"

HRESULT fs_error::fs_hr(const int fs_code)
{
    return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200 + fs_code);
}

