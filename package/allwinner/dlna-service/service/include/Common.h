#ifndef __COMMON_H__
#define __COMMON_H__

#include <tina_log.h>

/*----------------------------------------------------------------------
|   result codes
+---------------------------------------------------------------------*/
/** Result indicating that the operation or call succeeded */
#define SUCCESS                     0

/** Result indicating an unspecififed failure condition */
#define FAILURE                     (-1)

#include <assert.h>
#define ASSERT(x) assert(x)

/*----------------------------------------------------------------------
|   error code
+---------------------------------------------------------------------*/
#if !defined(ERROR_BASE)
#define ERROR_BASE -20000
#endif

// general errors
#define INVALID_PARAMETERS  (ERROR_BASE - 0)
#define PERMISSION_DENIED   (ERROR_BASE - 1)
#define OUT_OF_MEMORY       (ERROR_BASE - 2)
#define NO_SUCH_NAME        (ERROR_BASE - 3)
#define NO_SUCH_PROPERTY    (ERROR_BASE - 4)
#define NO_SUCH_ITEM        (ERROR_BASE - 5)
#define NO_SUCH_CLASS       (ERROR_BASE - 6)
#define OVERFLOW            (ERROR_BASE - 7)
#define INTERNAL            (ERROR_BASE - 8)
#define INVALID_STATE       (ERROR_BASE - 9)
#define INVALID_FORMAT      (ERROR_BASE - 10)
#define INVALID_SYNTAX      (ERROR_BASE - 11)
#define NOT_IMPLEMENTED     (ERROR_BASE - 12)
#define NOT_SUPPORTED       (ERROR_BASE - 13)
#define TIMEOUT             (ERROR_BASE - 14)
#define WOULD_BLOCK         (ERROR_BASE - 15)
#define TERMINATED          (ERROR_BASE - 16)
#define OUT_OF_RANGE        (ERROR_BASE - 17)
#define OUT_OF_RESOURCES    (ERROR_BASE - 18)
#define NOT_ENOUGH_SPACE    (ERROR_BASE - 19)
#define INTERRUPTED         (ERROR_BASE - 20)
#define CANCELLED           (ERROR_BASE - 21)

const char* ResultText(int result);

#define CHECK(_x)                   \
do {                                \
    int _result = (_x);             \
    if (_result != SUCCESS) {       \
        TLOGE("%s(%d): @@@ CHECK failed, result=%d (%s)\n", __FILE__, __LINE__, _result, ResultText(_result)); \
        return _result;             \
    }                               \
} while(0)

#define CHECK_POINTER(_p)                     \
do {                                          \
    if ((_p) == NULL) {                       \
        TLOGE("%s(%d): @@@ NULL pointer parameter\n", __FILE__, __LINE__); \
        return INVALID_PARAMETERS;            \
    }                                         \
} while(0)

#define DELETE_POINTER(_p)          \
do{                                 \
    if((_p) != NULL){               \
        TLOGD("delete: %p",_p);     \
        delete _p;                  \
        _p = NULL;                  \
    }                               \
}while(0)

#endif /*__COMMON_H__*/
