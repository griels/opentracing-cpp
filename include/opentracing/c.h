#ifndef OPENTRACING_C_H
#define OPENTRACING_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>

typedef struct opentracing_tracer_t opentracing_tracer_t;
typedef struct opentracing_span_t opentracing_span_t;
typedef struct opentracing_spancontext_t opentracing_spancontext_t;

typedef enum {
    opentracing_spanreferencetype_childofref = 1,
    opentracing_spanreferencetype_followsfromref = 2
} opentracing_spanreferencetype_t;

typedef enum {
    opentracing_invalid_span_context_error = 1,
    opentracing_invalid_carrier_context_error = 2,
    opentracing_span_context_corrupted_error = 3
} opentracing_error_code_t;

typedef struct opentracing_spanreference_t {
    opentracing_spanreferencetype_t type;
    const opentracing_spancontext_t* context;
} opentracing_spanreferencelist_t;

typedef struct opentracing_value_t opentracing_value_t;

typedef enum {
    opentracing_valuetype_bool,
    opentracing_valuetype_double,
    opentracing_valuetype_int64,
    opentracing_valuetype_uint64,
    opentracing_valuetype_string,
    opentracing_valuetype_unknown = -1
} opentracing_valuetype_t;

opentracing_valuetype_t opentracing_value_type(
    const opentracing_value_t* value);

opentracing_value_t* opentracing_new_value();

void opentracing_free_value(opentracing_value_t* value);

bool opentracing_get_value_bool(const opentracing_value_t* value);

double opentracing_get_value_double(const opentracing_value_t* value);

int64_t opentracing_get_value_int64(const opentracing_value_t* value);

uint64_t opentracing_get_value_uint64(const opentracing_value_t* value);

const char* opentracing_get_value_string(const opentracing_value_t* value);

void opentracing_set_value_bool(opentracing_value_t* value, bool val);

void opentracing_set_value_double(opentracing_value_t* value, double val);

void opentracing_set_value_int64(opentracing_value_t* value, int64_t val);

void opentracing_set_value_uint64(opentracing_value_t* value, uint64_t val);

void opentracing_set_value_string(
    opentracing_value_t* value, const char* val);

typedef struct opentracing_tag_t {
    const char* key;
    opentracing_value_t* value;
} opentracing_tag_t;

typedef struct opentracing_startspanoptions_t {
    struct timespec start_steady_timestamp;
    struct timespec start_system_timestamp;
    const opentracing_spanreference_t** references;
    int num_references;
    const opentracing_tag_t** tags;
    int num_tags;
} opentracing_startspanoptions_t;

typedef struct opentracing_finishspanoptions_t {
    struct timespec finish_timestamp;
} opentracing_finishspanoptions_t;

void opentracing_foreach_baggage_item(
    const opentracing_spancontext_t* span_context,
    bool (*callback_fn)(const char*, const char*, void*),
    void* context);

void opentracing_finish_span_with_options(
    opentracing_span_t* span,
    const opentracing_finishspanoptions_t* options);

void opentracing_finish_span(opentracing_span_t* span);

void opentracing_set_operation_name(
    opentracing_span_t* span, const char* operation_name);

void opentracing_set_tag(
    opentracing_span_t* span,
    const char* key,
    const opentracing_value_t* value);

void opentracing_set_baggage_item(
    opentracing_span_t* span,
    const char* key,
    const char* value);

const char* opentracing_baggage_item(
    opentracing_span_t* span,
    const char* key);

void opentracing_log(
    opentracing_span_t* span,
    const opentracing_tag_t fields[]);

void opentracing_context_from_span(
    opentracing_spancontext_t* span_context,
    const opentracing_span_t* span);

void opentracing_tracer_from_span(
    opentracing_tracer_t* tracer,
    const opentracing_span_t* span);

opentracing_span_t* opentracing_start_span(
    opentracing_tracer_t* tracer,
    const char* operation_name);

opentracing_span_t* opentracing_start_span_with_options(
    opentracing_tracer_t* tracer,
    const char* operation_name,
    const opentracing_startspanoptions_t* options);

void opentracing_free_span(opentracing_span_t* span);

typedef struct opentracing_buffer_t {
    char* data;
    int size;
} opentracing_buffer_t;

int opentracing_inject_binary(
    const opentracing_tracer_t* tracer,
    const opentracing_spancontext_t* sc,
    opentracing_buffer_t* buffer);

int opentracing_extract_binary(
    const opentracing_tracer_t* tracer,
    opentracing_spancontext_t* sc,
    const opentracing_buffer_t* buffer);

void opentracing_close_tracer(opentracing_tracer_t* tracer);

void opentracing_global_tracer(opentracing_tracer_t* tracer);

/* TODO
opentracing_tracer_t* opentracing_init_global_tracer(
    opentracing_tracer_t* tracer); */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* OPENTRACING_C_H */
