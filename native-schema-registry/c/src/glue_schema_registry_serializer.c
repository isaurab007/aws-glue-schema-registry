#include "glue_schema_registry_serializer.h"
#include "memory_allocator.h"
#include "glue_schema_registry_config.h"
#include "libnativeschemaregistry.h"
#include <stdlib.h>

glue_schema_registry_serializer *new_glue_schema_registry_serializer(glue_schema_registry_error **p_err) {
    glue_schema_registry_serializer *serializer = NULL;
    serializer = (glue_schema_registry_serializer *) aws_common_malloc(sizeof(glue_schema_registry_serializer));

    // Load YAML configuration
    gsr_yaml_config* yaml_config = load_yaml_config();
    if (!yaml_config) {
        delete_glue_schema_registry_serializer(serializer);
        throw_error(p_err, "Failed to load GSR configuration", ERR_CODE_CONFIG_LOAD_FAILED);
        return NULL;
    }

    // Initializes a GraalVM instance to call the entry points.
    int ret = graal_create_isolate(NULL, NULL, (graal_isolatethread_t **) &serializer->instance_context);

    if (ret != 0) {
        free_yaml_config(yaml_config);
        delete_glue_schema_registry_serializer(serializer);
        throw_error(p_err, "Failed to initialize GraalVM isolate.", ERR_CODE_GRAALVM_INIT_EXCEPTION);
        return NULL;
    }
    
    // Initialize serializer with YAML configuration
    initialize_serializer_with_config(
        serializer->instance_context,
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
    return serializer;
}

void delete_glue_schema_registry_serializer(glue_schema_registry_serializer *serializer) {
    //Tear down the GraalVM instance.
    if (serializer == NULL) {
        log_warn("Serializer is NULL", ERR_CODE_NULL_PARAMETERS);
        return;
    }
    if (serializer->instance_context != NULL) {
        int ret = graal_tear_down_isolate(serializer->instance_context);
        if (ret != 0) {
            log_warn("Error tearing down the graal isolate instance.", ERR_CODE_GRAALVM_TEARDOWN_EXCEPTION);
        }
        serializer->instance_context = NULL;
    }

    aws_common_free(serializer);
}

mutable_byte_array *glue_schema_registry_serializer_encode(
        glue_schema_registry_serializer *serializer,
        read_only_byte_array *array,
        const char * transport_name,
        glue_schema_registry_schema *gsr_schema,
        glue_schema_registry_error **p_err) {
    if (serializer == NULL || serializer->instance_context == NULL) {
        throw_error(p_err, "Serializer instance or instance context is null.", ERR_CODE_INVALID_STATE);
        return NULL;
    }

    if (gsr_schema == NULL) {
        throw_error(p_err, "Schema passed cannot be null", ERR_CODE_NULL_PARAMETERS);
        return NULL;
    }

    if (array == NULL || array->len == 0) {
        throw_error(p_err, "Byte array cannot be null", ERR_CODE_NULL_PARAMETERS);
        return NULL;
    }

    return encode_with_schema(serializer->instance_context, array, transport_name, gsr_schema, p_err);
}
