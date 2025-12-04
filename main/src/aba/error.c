#include "aba/error.h"

const char *_ABA_ERR_STRINGS[] = {
#define X(name) #name,
    _ABA_ERR_LIST
#undef X
};

const char *abaErrToStr(AbaErr err)
{
    if (ABA_ERR_NONE <= err && err < _ABA_ERR_END)
        return _ABA_ERR_STRINGS[err];
    return "HY_ERR_UNKNOWN";
}