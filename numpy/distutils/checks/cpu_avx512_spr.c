#if defined(DETECT_FEATURES) && defined(__INTEL_COMPILER)
    /*
     * Unlike GCC and CLANG, Intel Compiler exposes all supported intrinsics,
     * whether or not the build options for those features are specified.
     * Therefore, we must test #definitions of CPU features when option native/host
     * is enabled via `--cpu-baseline` or through env var `CFLAGS` otherwise
     * the test will be broken and leads to enable all possible features.
     */
    #if !defined(__AVX512VL__) || !defined(__AVX512BW__) || !defined(__AVX512DQ__) || !defined(__AVX512FP16__)
        #error "HOST/ARCH doesn't support Sapphire Rapids AVX512 features"
    #endif
#endif

#include <immintrin.h>

int main(int argc, char **argv)
{
    __m512i aa = _mm512_abs_epi32(_mm512_loadu_si512((const __m512i*)argv[argc-1]));
    /* VL */
    __m256i a = _mm256_abs_epi64(_mm512_extracti64x4_epi64(aa, 1));
    /* DQ */
    __m512i b = _mm512_broadcast_i32x8(a);
    /* BW */
    b = _mm512_abs_epi16(b);
    /* FP16 */
    __m256h temp = _mm512_cvtepi32_ph(b);
    _mm256_storeu_epi32((void*)(argv[argc-1]), _mm256_castph_si256(temp));
    return 0;
}
