#ifndef GLUE_SCHEMA_REGISTRY_CONFIG_H
#define GLUE_SCHEMA_REGISTRY_CONFIG_H

#include <stdbool.h>

/**
 * Structure to hold GSR configuration loaded from YAML (P0 features only)
 */
typedef struct gsr_yaml_config {
    // AWS & Core Settings (P0)
    char* aws_region;
    char* aws_endpoint;
    char* registry_name;
    char* schema_name;
    
    // Schema Management (P0)
    bool schema_auto_registration;
    char* compatibility_setting;
    char* description;
    
    // Data Format Settings (P0)
    char* data_format;
    char* protobuf_message_type;
    
    // Performance & Caching (P0)
    int cache_size;
    long cache_ttl_millis;
    
    // Advanced Features (P0)
    char* compression_type;
    char* secondary_deserializer;
} gsr_yaml_config;

/**
 * Load configuration from YAML file
 * Searches for config file in the following order:
 * 1. Path specified by GLUE_SCHEMA_REGISTRY_CONFIG environment variable
 * 2. ./gsr-config.yaml (current directory)
 * 3. ~/.aws/gsr-config.yaml (user home directory)
 * 4. /etc/aws/gsr-config.yaml (system directory)
 * 
 * @return Pointer to configuration structure or NULL if not found
 */
gsr_yaml_config* load_yaml_config();

/**
 * Free memory allocated for configuration
 * 
 * @param config Pointer to configuration structure
 */
void free_yaml_config(gsr_yaml_config* config);

/**
 * Get default configuration when no YAML file is found
 * 
 * @return Pointer to configuration structure with default values
 */
gsr_yaml_config* get_default_config();

#endif /* GLUE_SCHEMA_REGISTRY_CONFIG_H */