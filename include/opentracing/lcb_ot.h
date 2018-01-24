//
// Created by Ellis Breen on 23/01/2018.
//

#ifndef OPENTRACING_CPP_LCB_OT_H
#define OPENTRACING_CPP_LCB_OT_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct opentracing_string_t {
    const char* data;
    int size;
} opentracing_string_t;

#ifdef OT_STR_GEN
#error OT_STR_GEN defined
#endif

#define OT_STR_GEN(X) { static opentracing_string_t result={#X,sizeof(#X)/sizeof(char)}; return &result; };

const char* ot_str(const opentracing_string_t* string)
{
    return string->data;
}

#define PP_EACH_SPAN_ID(MAND,OPT,DIV)\
    OPT(RequestQueue)##DIV\
    MAND(RequestEncoding)##DIV\
    MAND(DispatchToServer)##DIV\
    MAND(ResponseDecoding)##DIV\
    OPT(ResponseResolution)

typedef struct lcb_span_id_t {
    const char* reserved;
    enum id {
        #define MAND(X) X
        #define OPT(X) X
        #define DIV ,
            PP_EACH_SPAN_ID(MAND,OPT,DIV)
        #undef DIV
        #undef OPT
        #undef MAND
    };
} lcb_span_id_t;

const opentracing_string_t* lcb_ot_id_str(lcb_span_id_t::id id)
{
    switch(id)
    {
        #define MAND(X)\
            case lcb_span_id_t::X:\
                OT_STR_GEN(X);
        #define OPT(X) MAND(X)
        #define DIV
            break;
        PP_EACH_SPAN_ID(MAND,OPT,DIV);
        #undef DIV
        #undef OPT
        #undef MAND
            default:
                OT_STR_GEN("");
    }
}

struct lcb_tag_id_t
{
    #ifdef PP_EACH_TAG_ID
    #error PP_EACH_TAG_ID defined already
    #endif
    #define PP_EACH_TAG_ID(NAMESPACE_FN,DIV)\
        NAMESPACE_FN(couchbase,operation_id,service)##DIV\
        NAMESPACE_FN(local,address)##DIV\
        NAMESPACE_FN(peer,address)

    struct ns {
        #define DIV ,
        #define NS_ENUM(X,...) X
            enum { PP_EACH_TAG_ID(NS_ENUM,DIV) };
        #undef NS_ENUM
        #undef DIV
    };
    union tag_t {
        #define TAG_TYPE(NAMESPACE,...)\
            struct NAMESPACE\
            {\
                enum id\
                {\
                    __VA_ARGS__\
                };\
            };
        #define DIV
            PP_EACH_TAG_ID(TAG_TYPE,DIV);
        #undef DIV
        #undef TAG_TYPE
    } ;
    ns a;
    tag_t b;
    #undef PP_EACH_TAG_ID

};

#undef OT_STR_GEN

void test()
{
    lcb_span_id_t test;
    printf(ot_str(lcb_ot_id_str(lcb_span_id_t::DispatchToServer)));
}

void func()
{
    lcb_tag_id_t tag={lcb_tag_id_t::ns::couchbase, lcb_tag_id_t::tag_t::couchbase::operation_id};
}

#define LCB_OT_STR_UNION(type) \
typedef union lcb_opentracing_##type##_t {\
    opentracing_string_t name;\
    lcb_##type##_t id;\
} lcb_opentracing_##type##_t;

LCB_OT_STR_UNION(tag_id);
LCB_OT_STR_UNION(span_id);

typedef struct opentracing_tracer_t opentracing_tracer_t;


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

#define LCB_OPENTRACING_DICT(type)\
struct lcb_opentracing_dictionary_##type##_t {\
    type key;\
    opentracing_value_t value;\
    lcb_opentracing_dictionary_##type##_t* next;\
}

typedef LCB_OPENTRACING_DICT(lcb_opentracing_tag_id_t) lcb_opentracing_tags_t;
typedef struct lcb_opentracing_start_span_options_t {
    struct timespec start_timestamp;
    const opentracing_span_references_t* references;
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
    const opentracing_span_context_t* (*get_context)(const void* self);
    const opentracing_tracer_t* (*get_tracer)(const void* self);
} lcb_opentracing_span_t;

typedef struct lcb_opentracing_start_span_options_t {
    struct timespec start_timestamp;
    const opentracing_span_references_t* references;
    const lcb_opentracing_tags_t* tags;
} lcb_opentracing_start_span_options_t;

struct lcb_opentracing_tracer_t {
    void (*destructor)(void* self);
    lcb_opentracing_span_t* (*start_span_with_options)(
            const void* self,
            const opentracing_string_t* operation_name,
            const lcb_opentracing_start_span_options_t* options);

    void (*close)(void* self);
};

#endif //OPENTRACING_CPP_LCB_OT_H
