#ifndef GLUE_SCHEMA_REGISTRY_CONFIG_H
#define GLUE_SCHEMA_REGISTRY_CONFIG_H

#include "glue_schema_registry_error.h"

/**
 * Sets the configuration file path for the schema registry.
 * 
 * @param config_file_path Path to the configuration file
 * @param p_err Pointer to error object that will be set if an error occurs
 * @return 0 if successful, non-zero otherwise
 */
int glue_schema_registry_set_config_file(const char* config_file_path, glue_schema_registry_error **p_err);

#endif // GLUE_SCHEMA_REGISTRY_CONFIG_H