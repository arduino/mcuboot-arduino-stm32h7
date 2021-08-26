#if !defined(MBEDTLS_AES_C)
#define MBEDTLS_AES_C
#endif

#if !defined(MBEDTLS_CIPHER_MODE_CTR)
#define MBEDTLS_CIPHER_MODE_CTR
#endif

#if defined (MBEDTLS_SHA1_C)
#undef MBEDTLS_SHA1_C
#endif
