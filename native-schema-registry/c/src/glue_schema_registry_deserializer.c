#include "glue_schema_registry_deserializer.h"
#include "memory_allocator.h"
#include "glue_schema_registry_config.h"
#include "libnativeschemaregistry.h"
#include <stdlib.h>

glue_schema_registry_deserializer * new_glue_schema_registry_deserializer(glue_schema_registry_error **p_err) {
    glue_schema_registry_deserializer *deserializer = NULL;
    deserializer =
            (glue_schema_registry_deserializer *) aws_common_malloc(sizeof(glue_schema_registry_deserializer));

    // Load YAML configuration
    gsr_yaml_config* yaml_config = load_yaml_config();
    if (!yaml_config) {
        delete_glue_schema_registry_deserializer(deserializer);
        throw_error(p_err, "Failed to load GSR configuration", ERR_CODE_CONFIG_LOAD_FAILED);
        return NULL;
    }

    int ret = graal_create_isolate(NULL, NULL, (graal_isolatethread_t **) &deserializer->instance_context);
    if (ret != 0) {
        free_yaml_config(yaml_config);
        delete_glue_schema_registry_deserializer(deserializer);
        throw_error(p_err, "Failed to initialize GraalVM isolate.", ERR_CODE_GRAALVM_INIT_EXCEPTION);
        return NULL;
    }
    
    // Initialize deserializer with YAML configuration
    initialize_deserializer_with_config(
        deserializer->instance_context,
        yaml_config->aws_region,
        yaml_config->aws_endpoint,
        yaml_config->registry_name,
        yaml_config->schema_name,
        yaml_config->schema_auto_registration ? 1 : 0,
        yaml_config->compatibility_setting,
        yaml_config->description,
        yaml_config->cache_size,
        yaml_config->cache_ttl_millis,
        yaml_config->compression_type,
        yaml_config->secondary_deserializer,
        yaml_config->data_format,
        yaml_config->protobuf_message_type
    );
    
    free_yaml_config(yaml_config);
    return deserializer;
}

void delete_glue_schema_registry_deserializer(glue_schema_registry_deserializer * deserializer) {
    if (deserializer == NULL) {
        log_warn("Deserializer is NULL", ERR_CODE_NULL_PARAMETERS);
        return;
    }
    if (deserializer->instance_context != NULL) {
        int ret = graal_tear_down_isolate(deserializer->instance_context);
        if (ret != 0) {
            log_warn("Error tearing down the graal isolate instance.", ERR_CODE_GRAALVM_TEARDOWN_EXCEPTION);
        }
        deserializer->instance_context = NULL;
    }

    aws_common_free(deserializer);
}

static bool validate(
        glue_schema_registry_deserializer *deserializer,
        read_only_byte_array *array,
        glue_schema_registry_error **p_err) {
    if (deserializer == NULL || deserializer->instance_context == NULL) {
        throw_error(p_err, "Deserializer instance or instance context is null.", ERR_CODE_INVALID_STATE);
        return false;
    }

    if (array == NULL || array->len == 0) {
        throw_error(p_err, "Byte array cannot be null", ERR_CODE_NULL_PARAMETERS);
        return false;
    }
    return true;
}

mutable_byte_array *glue_schema_registry_deserializer_decode(glue_schema_registry_deserializer * deserializer,
                                                             read_only_byte_array *array,
                                                             glue_schema_registry_error **p_err) {
    if (!validate(deserializer, array, p_err)) {
        return NULL;
    }

    return decode(deserializer->instance_context, array, p_err);
}

glue_schema_registry_schema *glue_schema_registry_deserializer_decode_schema(glue_schema_registry_deserializer * deserializer,
                                                                             read_only_byte_array *array,
                                                                             glue_schema_registry_error **p_err) {
    if (!validate(deserializer, array, p_err)) {
        return NULL;
    }

    glue_schema_registry_schema * schema = decode_schema(deserializer->instance_context, array, p_err);
    return schema;
}

bool glue_schema_registry_deserializer_can_decode(glue_schema_registry_deserializer * deserializer,
                                                  read_only_byte_array *array,
                                                  glue_schema_registry_error **p_err) {
    if (!validate(deserializer, array, p_err)) {
        return NULL;
    }

    return can_decode(deserializer->instance_context, array, p_err);
}

