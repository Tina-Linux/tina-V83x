#include <wchar.h>
#include "fortify.h"

wchar_t *__wcscpy_chk(wchar_t *restrict d, const wchar_t *restrict s, size_t dn)
{
  return wcscpy(d, s);
}