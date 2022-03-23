#include <wchar.h>
#include "fortify.h"

wchar_t *__wcscat_chk(wchar_t *restrict dest, const wchar_t *restrict src, size_t destlen) {
	return wcscat(dest, src);
}