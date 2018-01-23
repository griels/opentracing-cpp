//
// Created by Ellis Breen on 23/01/2018.
//

#ifndef OPENTRACING_CPP_LCB_OT_H
#define OPENTRACING_CPP_LCB_OT_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct opentracing_tracer_t opentracing_tracer_t;

typedef struct lcb_opentracing_string_t {
    const char* data;
    int size;
} lcb_opentracing_string_t;

typedef struct lcb_span_id_t {
    enum {RequestQueue, DispatchToServer,
        ResponseDecoding,
        ResponseResolution} id;
} lcb_span_id;

typedef struct lcb_tag_id_t {
    enum {couchbase__operation_id,
        couchbase__service,
        local__address,
        peer__address,
    } id;
} lcb_tag_id_t;

typedef union lcb_opentracing_span_id_t {
    lcb_opentracing_string_t name;
    lcb_span_id_t id;
} opentracing_string_t;

typedef union lcb_opentracing_tag_id_t {
    lcb_opentracing_string_t name;
    lcb_tag_id_t id;
} opentracing_string_t;

typedef bool(*opentracing_foreach_key_value_callback_t)(
        const opentracing_string_t* key, const opentracing_string_t* value);

typedef struct opentracing_span_context_t {
    void (*destructor)(void* self);
    void (*foreach_baggage_item)(void* self,
                                 opentracing_foreach_key_value_callback_t f);
} opentracing_span_context_t;

typedef enum opentracing_span_reference_type_t {
    opentracing_span_reference_child_of_ref = 1,
    opentracing_span_reference_follows_from_ref = 2
} opentracing_span_reference_type_t;
typedef struct opentracing_span_references_t {
    opentracing_span_reference_type_t type;
    const opentracing_span_context_t* context;
    struct opentracing_span_references_t* next;
} opentracing_span_references_t;

typedef enum opentracing_value_index_t {
    opentracing_value_index_bool,
    opentracing_value_index_double,
    opentracing_value_index_int64,
    opentracing_value_index_uint64,
    opentracing_value_index_string_buffer,
    opentracing_value_index_string,
    opentracing_value_index_values,
    opentracing_value_index_dictionary,
    opentracing_value_index_null = -1
} opentracing_value_index_t;

typedef struct opentracing_values_t opentracing_values_t;

typedef struct opentracing_dictionary_t opentracing_dictionary_t;
typedef struct opentracing_string_buffer_t {
    char* data;
    int size;
    int capacity;
} opentracing_string_buffer_t;

typedef struct opentracing_value_t {
    opentracing_value_index_t value_index;
    union {
        bool bool_value;
        double double_value;
        int64_t int64_value;
        uint64_t uint64_value;
        opentracing_string_buffer_t string_buffer_value;
        opentracing_string_t string_value;
        opentracing_values_t* list_value;
        opentracing_dictionary_t* dict_value;
    } data;
} opentracing_value_t;

struct lcb_opentracing_dictionary_t {
    lcb_opentracing_string_t key;
    lcb_opentracing_value_t value;
    lcb_opentracing_dictionary_t* next;
};

typedef lcb_opentracing_dictionary_t lcb_opentracing_tags_t;
typedef struct lcb_opentracing_start_span_options_t {
    struct timespec start_timestamp;
    const lcb_opentracing_span_references_t* references;
    const lcb_opentracing_tags_t* tags;
} lcb_opentracing_start_span_options_t;


typedef struct lcb_opentracing_finish_span_options_t {
    struct timespec finish_timestamp;
} lcb_opentracing_finish_span_options_t;

typedef struct lcb_opentracing_span_t {
    void (*destructor)(void* self);
    void (*finish)(void* self,
                   const lcb_opentracing_finish_span_options_t* options);
    void (*set_operation_id)(void* self,
                               const lcb_opentracing_span_id_t* name);
    void (*set_tag)(void* self,
                    const lcb_opentracing_tag_id_t* key,
                    const opentracing_value_t* value);

    void (*log)(void* self,
                const opentracing_dictionary_t* fields);
    const lcb_opentracing_span_context_t* (*get_context)(const void* self);
    const lcb_opentracing_tracer_t* (*get_tracer)(const void* self);
} lcb_opentracing_span_t;

typedef struct lcb_opentracing_start_span_options_t {
    struct timespec start_timestamp;
    const lcb_opentracing_span_references_t* references;
    const lcb_opentracing_tags_t* tags;
} lcb_opentracing_start_span_options_t;

struct lcb_opentracing_tracer_t {
    void (*destructor)(void* self);
    lcb_opentracing_span_t* (*start_span_with_options)(
            const void* self,
            const opentracing_string_t* operation_name,
            const opentracing_start_span_options_t* options);

    void (*close)(void* self);
};

#endif //OPENTRACING_CPP_LCB_OT_H
