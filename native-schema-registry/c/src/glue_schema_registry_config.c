#include "../include/glue_schema_registry_config.h"
#include "../include/glue_schema_registry_error.h"
#include "libnativeschemaregistry.h"

int glue_schema_registry_set_config_file(const char* config_file_path, glue_schema_registry_error **p_err) {
    int result = ReadConfigurationFile(config_file_path);
    
    if (result != 0) {
        *p_err = new_glue_schema_registry_error("Failed to read configuration file");
        return result;
    }
    
    return 0;
}