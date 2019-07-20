#define _UMATHMODULE
#define _MULTIARRAYMODULE
#define NPY_NO_DEPRECATED_API NPY_API_VERSION

#include <Python.h>

#include "npy_config.h"

#include "cpuid.h"

#define XCR_XFEATURE_ENABLED_MASK 0x0
#define XSTATE_SSE 0x2
#define XSTATE_YMM 0x4
#define XSTATE_ZMM 0x70

static NPY_INLINE
void npy_cpuid(uint32_t leaf,
               uint32_t* eax,
               uint32_t* ebx,
               uint32_t* ecx,
               uint32_t* edx)
{
#if defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64)
#if defined __GNUC__
    __asm__ volatile (
      "cpuid"				"\n\t"
	: "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
	: "a" (leaf), "c" (0));
#elif defined (_MSC_VER)
    int info[4];
    __cpuid (info, leaf);
    *eax = info[0];
    *ebx = info[1];
    *ecx = info[2];
    *edx = info[3];
#endif
#endif
}

/*
 * verify the OS supports avx instructions
 * it can be disabled in some OS, e.g. with the nosavex boot option of linux
 */
static NPY_INLINE
int os_avx_support(void)
{
#if HAVE_XGETBV
    /*
     * use bytes for xgetbv to avoid issues with compiler not knowing the
     * instruction
     */
    unsigned int eax, edx;
    unsigned int ecx = XCR_XFEATURE_ENABLED_MASK;
    __asm__("xgetbv" : "=a" (eax), "=d" (edx) : "c" (ecx));
    return (eax & (XSTATE_SSE | XSTATE_YMM)) == (XSTATE_SSE | XSTATE_YMM);
#else
    return 0;
#endif
}

static NPY_INLINE
int os_avx512_support(void)
{
#if HAVE_XGETBV
    unsigned int eax, edx;
    unsigned int ecx = XCR_XFEATURE_ENABLED_MASK;
    unsigned int xcr0 = XSTATE_ZMM | XSTATE_YMM | XSTATE_SSE;
    __asm__("xgetbv" : "=a" (eax), "=d" (edx) : "c" (ecx));
    return (eax & xcr0) == xcr0;
#else
    return 0;
#endif
}

static NPY_INLINE
uint32_t cpuid_supports_avx512f(void)
{
    /*
     * Call 7th leaf of CPUID
     * CPUID sets the 17th bit of EBX register if AVX512F is supported
     */
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    uint32_t leaf = 0x07;
    npy_cpuid(leaf, &eax, &ebx, &ecx, &edx);
    return (ebx >> 16) & 0x01;
}
/*
 * Primitive cpu feature detect function
 * Currently only supports checking for avx on gcc compatible compilers.
 */
NPY_NO_EXPORT int
npy_cpu_supports(const char * feature)
{
#ifdef HAVE___BUILTIN_CPU_SUPPORTS
    if (strcmp(feature, "avx512f") == 0) {
#if defined(__GNUC__) && (__GNUC__ < 5)
        return 0;
#else
        return cpuid_supports_avx512f() && os_avx512_support();
#endif
    }
    else if (strcmp(feature, "avx2") == 0) {
        return __builtin_cpu_supports("avx2") && os_avx_support();
    }
    else if (strcmp(feature, "avx") == 0) {
        return __builtin_cpu_supports("avx") && os_avx_support();
    }
#endif

    return 0;
}
