#ifndef __ABA_ERROR_H
#define __ABA_ERROR_H

#define _ABA_ERR_LIST        \
    X(ABA_ERR_NONE)          \
    X(ABA_ERR_BAD_ARGS)      \
    X(ABA_ERR_MALLOC_FAILED) \
    X(ABA_ERR_FAILURE)       \
    X(ABA_ERR_TIMEOUT)

extern const char *_ABA_ERR_STRINGS[];

typedef enum
{
#define X(name) name,
    _ABA_ERR_LIST
#undef X
        _ABA_ERR_END
} AbaErr;

const char *abaErrToStr(AbaErr err);

#endif