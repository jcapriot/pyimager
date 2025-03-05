from libc.stdint cimport uint16_t, uint32_t, uint64_t

cdef extern from "bswap.h" nogil:

    uint16_t spy_bswap_u16(uint32_t x)
    uint32_t spy_bswap_u32(uint32_t x)
    uint64_t spy_bswap_u64(uint32_t x)

    void swap16_big_and_system(uint16_t *x, size_t n)
    void swap32_big_and_system(uint32_t *x, size_t n)
    void swap64_big_and_system(uint64_t *x, size_t n)
    void swap16_little_and_system(uint16_t *x, size_t n)
    void swap32_little_and_system(uint32_t *x, size_t n)
    void swap64_little_and_system(uint64_t *x, size_t n)
    void swap16_pairwise_and_system(uint16_t *x, size_t n)
    void swap32_pairwise_and_system(uint32_t *x, size_t n)
    void swap64_pairwise_and_system(uint64_t *x, size_t n)

    void swap_struct_big_and_system(char *str, size_t *sizes, size_t n_attr)
    void swap_struct_little_and_system(char *str, size_t *sizes, size_t n_attr)
    void swap_struct_pairwise_and_system(char *str, size_t *sizes, size_t n_attr)