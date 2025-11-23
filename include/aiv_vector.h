#ifndef AIV_VECTOR
#define AIV_VECTOR

#include <stdbool.h>

#ifdef _WIN32
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT
#endif

typedef struct aiv_vector
{
    void** items;
    size_t count;
    size_t capacity;
} aiv_vector_t;

DLL_EXPORT aiv_vector_t aiv_vector_new();
DLL_EXPORT void aiv_vector_destroy(aiv_vector_t* vector);
DLL_EXPORT void aiv_vector_add(aiv_vector_t* vector, void* item);
DLL_EXPORT void aiv_vector_remove(aiv_vector_t* vector, void* item, int (*comparator)(void* item1, void* item2));
DLL_EXPORT void aiv_vector_insert_at(aiv_vector_t* vector, void* item, size_t index);
DLL_EXPORT void aiv_vector_remove_at(aiv_vector_t* vector, size_t index);
DLL_EXPORT void* aiv_vector_at(aiv_vector_t* vector, size_t index);
DLL_EXPORT bool aiv_vector_is_empty(aiv_vector_t* vector);
DLL_EXPORT size_t aiv_vector_size(aiv_vector_t* vector);
// set
DLL_EXPORT void aiv_vector_bubble(aiv_vector_t* vector, int (*comparator)(void* item1, void* item2));
// quick_sort

#endif // AIV_VECTOR