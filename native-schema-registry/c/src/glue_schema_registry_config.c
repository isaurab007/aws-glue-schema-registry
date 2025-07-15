#include "glue_schema_registry_config.h"
#include "glue_schema_registry_error.h"
#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

// Constants to avoid magic numbers
#define MAX_PATH_LENGTH 1024
#define DEFAULT_CACHE_SIZE 200
#define SECONDS_PER_HOUR 3600
#define HOURS_PER_DAY 24
#define MILLISECONDS_PER_SECOND 1000

/**
 * Find configuration file in standard locations
 */
static char* find_config_file() {
    // Priority order for config file discovery
    const char* env_path = getenv("GLUE_SCHEMA_REGISTRY_CONFIG");
    if (env_path && access(env_path, F_OK) == 0) {
        return strdup(env_path);
    }
    
    // Check current directory
    if (access("./gsr-config.yaml", F_OK) == 0) {
        return strdup("./gsr-config.yaml");
    }
    
    // Check user home directory
    const char* home_dir = getenv("HOME");
    if (!home_dir) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) {
            home_dir = pwd->pw_dir;
        }
    }
    
    if (home_dir) {
        char home_path[MAX_PATH_LENGTH];
        snprintf(home_path, sizeof(home_path), "%s/.aws/gsr-config.yaml", home_dir);
        if (access(home_path, F_OK) == 0) {
            return strdup(home_path);
        }
    }
    
    // Check system directory
    if (access("/etc/aws/gsr-config.yaml", F_OK) == 0) {
        return strdup("/etc/aws/gsr-config.yaml");
    }
    
    return NULL;
}

/**
 * Helper function to duplicate string or return NULL
 */
static char* safe_strdup(const char* str) {
    return str ? strdup(str) : NULL;
}

/**
 * Parse a scalar value from YAML
 */
static char* get_scalar_value(yaml_document_t* document, yaml_node_t* node) {
    if (!node || node->type != YAML_SCALAR_NODE) {
        return NULL;
    }
    return safe_strdup((char*)node->data.scalar.value);
}

/**
 * Find a mapping node by key in a mapping
 */
static yaml_node_t* find_map_node(yaml_document_t* document, yaml_node_t* mapping, const char* key) {
    if (!mapping || mapping->type != YAML_MAPPING_NODE) {
        return NULL;
    }
    
    yaml_node_pair_t* pair;
    for (pair = mapping->data.mapping.pairs.start; pair < mapping->data.mapping.pairs.top; pair++) {
        yaml_node_t* key_node = yaml_document_get_node(document, pair->key);
        if (key_node && key_node->type == YAML_SCALAR_NODE && 
            strcmp((char*)key_node->data.scalar.value, key) == 0) {
            return yaml_document_get_node(document, pair->value);
        }
    }
    
    return NULL;
}

/**
 * Parse AWS section from YAML (P0)
 */
static void parse_aws_section(yaml_document_t* document, yaml_node_t* root, gsr_yaml_config* config) {
    yaml_node_t* aws_node = find_map_node(document, root, "aws");
    if (aws_node) {
        config->aws_region = get_scalar_value(document, find_map_node(document, aws_node, "region"));
        config->aws_endpoint = get_scalar_value(document, find_map_node(document, aws_node, "endpoint"));
    }
}

/**
 * Parse registry section from YAML (P0)
 */
static void parse_registry_section(yaml_document_t* document, yaml_node_t* root, gsr_yaml_config* config) {
    yaml_node_t* registry_node = find_map_node(document, root, "registry");
    if (registry_node) {
        config->registry_name = get_scalar_value(document, find_map_node(document, registry_node, "name"));
        
        yaml_node_t* auto_reg_node = find_map_node(document, registry_node, "schemaAutoRegistration");
        if (auto_reg_node && auto_reg_node->type == YAML_SCALAR_NODE) {
            const char* value = (char*)auto_reg_node->data.scalar.value;
            config->schema_auto_registration = 
                (strcmp(value, "true") == 0 || strcmp(value, "yes") == 0 || strcmp(value, "1") == 0);
        }
        
        config->compatibility_setting = get_scalar_value(document, find_map_node(document, registry_node, "compatibilitySetting"));
        config->description = get_scalar_value(document, find_map_node(document, registry_node, "description"));
    }
}

/**
 * Parse schema section from YAML (P0)
 */
static void parse_schema_section(yaml_document_t* document, yaml_node_t* root, gsr_yaml_config* config) {
    yaml_node_t* schema_node = find_map_node(document, root, "schema");
    if (schema_node) {
        config->schema_name = get_scalar_value(document, find_map_node(document, schema_node, "name"));
    }
}

/**
 * Parse cache section from YAML (P0)
 */
static void parse_cache_section(yaml_document_t* document, yaml_node_t* root, gsr_yaml_config* config) {
    yaml_node_t* cache_node = find_map_node(document, root, "cache");
    if (cache_node) {
        yaml_node_t* size_node = find_map_node(document, cache_node, "size");
        if (size_node && size_node->type == YAML_SCALAR_NODE) {
            config->cache_size = atoi((char*)size_node->data.scalar.value);
        }
        
        yaml_node_t* ttl_node = find_map_node(document, cache_node, "ttlMillis");
        if (ttl_node && ttl_node->type == YAML_SCALAR_NODE) {
            config->cache_ttl_millis = atol((char*)ttl_node->data.scalar.value);
        }
    }
}

/**
 * Parse data format section from YAML (P0)
 */
static void parse_dataformat_section(yaml_document_t* document, yaml_node_t* root, gsr_yaml_config* config) {
    yaml_node_t* df_node = find_map_node(document, root, "dataFormat");
    if (!df_node) {
        return;
    }
    
    // Get data format type (P0)
    yaml_node_t* format_node = find_map_node(document, df_node, "type");
    if (format_node && format_node->type == YAML_SCALAR_NODE) {
        config->data_format = safe_strdup((char*)format_node->data.scalar.value);
    }
    
    // Get Protobuf settings (P0)
    yaml_node_t* protobuf_node = find_map_node(document, df_node, "protobuf");
    if (protobuf_node) {
        config->protobuf_message_type = get_scalar_value(document, find_map_node(document, protobuf_node, "messageType"));
    }
}

/**
 * Parse advanced section from YAML (P0)
 */
static void parse_advanced_section(yaml_document_t* document, yaml_node_t* root, gsr_yaml_config* config) {
    yaml_node_t* advanced_node = find_map_node(document, root, "advanced");
    if (!advanced_node) {
        return;
    }
    
    // P0 features only
    config->compression_type = get_scalar_value(document, find_map_node(document, advanced_node, "compressionType"));
    config->secondary_deserializer = get_scalar_value(document, find_map_node(document, advanced_node, "secondaryDeserializer"));
}

/**
 * Load YAML configuration from file
 */
gsr_yaml_config* load_yaml_config() {
    char* config_path = find_config_file();
    if (!config_path) {
        return get_default_config();
    }
    
    FILE* file = fopen(config_path, "r");
    if (!file) {
        free(config_path);
        return get_default_config();
    }
    
    gsr_yaml_config* config = calloc(1, sizeof(gsr_yaml_config));
    
    // Parse YAML using libyaml
    yaml_parser_t parser;
    yaml_document_t document;
    
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);
    
    if (yaml_parser_load(&parser, &document)) {
        yaml_node_t* root = yaml_document_get_root_node(&document);
        
        // Parse YAML structure
        parse_aws_section(&document, root, config);
        parse_registry_section(&document, root, config);
        parse_schema_section(&document, root, config);
        parse_cache_section(&document, root, config);
        parse_dataformat_section(&document, root, config);
        parse_advanced_section(&document, root, config);
    }
    
    yaml_document_delete(&document);
    yaml_parser_delete(&parser);
    fclose(file);
    free(config_path);
    
    return config;
}

/**
 * Free memory allocated for configuration
 */
void free_yaml_config(gsr_yaml_config* config) {
    if (!config) {
        return;
    }
    
    // Free AWS & Core Settings
    free(config->aws_region);
    free(config->aws_endpoint);
    free(config->registry_name);
    free(config->schema_name);
    
    // Free Schema Management
    free(config->compatibility_setting);
    free(config->description);
    
    // Free Data Format Settings
    free(config->data_format);
    free(config->protobuf_message_type);
    
    // Free Advanced Features
    free(config->compression_type);
    free(config->secondary_deserializer);
    
    free(config);
}

/**
 * Get default configuration
 */
gsr_yaml_config* get_default_config() {
    gsr_yaml_config* config = calloc(1, sizeof(gsr_yaml_config));
    
    // Set default values for P0 features
    config->aws_region = safe_strdup("us-east-1");
    config->registry_name = safe_strdup("default-registry");
    config->schema_auto_registration = true;
    config->compatibility_setting = safe_strdup("BACKWARD");
    config->data_format = safe_strdup("PROTOBUF");
    config->cache_size = DEFAULT_CACHE_SIZE;
    config->cache_ttl_millis = HOURS_PER_DAY * SECONDS_PER_HOUR * MILLISECONDS_PER_SECOND; // 24 hours
    config->compression_type = safe_strdup("NONE");
    
    return config;
}