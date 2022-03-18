#if !defined(MBEDTLS_AES_C)
#define MBEDTLS_AES_C
#endif

#if !defined(MBEDTLS_CIPHER_MODE_CTR)
#define MBEDTLS_CIPHER_MODE_CTR
#endif

#if defined(MBEDTLS_SHA1_C)
#undef MBEDTLS_SHA1_C
#endif

#if defined(MBEDTLS_SHA512_C)
#undef MBEDTLS_SHA512_C
#endif

#if defined(MBEDTLS_AES_ROM_TABLES)
#undef MBEDTLS_AES_ROM_TABLES
#endif

#if defined(MBEDTLS_RSA_C)
#undef MBEDTLS_RSA_C
#endif

#if !defined(MBEDTLS_ECDH_C)
#define MBEDTLS_ECDH_C
#endif
