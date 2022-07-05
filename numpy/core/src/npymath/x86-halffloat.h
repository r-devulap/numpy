#include "numpy/npy_common.h"

#include "npy_cpu_dispatch.h"

#ifndef NPY_NO_EXPORT
#define NPY_NO_EXPORT NPY_VISIBILITY_HIDDEN
#endif

#ifndef NPY_DISABLE_OPTIMIZATION
#include "x86-halffloat.dispatch.h"
#endif

NPY_CPU_DISPATCH_DECLARE(NPY_NO_EXPORT npy_float x86_npy_half_to_float, (npy_half num))

NPY_CPU_DISPATCH_DECLARE(NPY_NO_EXPORT npy_half x86_npy_float_to_half, (npy_float num))
