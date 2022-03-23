#include <wchar.h>
#include "fortify.h"

wchar_t *__wcpcpy_chk(wchar_t *restrict d, const wchar_t *restrict s, size_t dn)
{
  return wcpcpy(d, s);
}