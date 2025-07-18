package com.amazonaws.services.schemaregistry;

import com.amazonaws.services.schemaregistry.common.configs.GlueSchemaRegistryConfiguration;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

/**
 * Reads configuration from a file and applies it to the Schema Registry.
 */
public class ConfigurationFileReader {

    /**
     * Reads configuration from a file and initializes the serializer and deserializer.
     *
     * @param thread GraalVM isolate thread
     * @param configFilePath Path to the configuration file
     * @return 0 if successful, non-zero otherwise
     */
    @CEntryPoint(name = "ReadConfigurationFile")
    public static int readConfigurationFile(IsolateThread thread, CCharPointer configFilePath) {
        try {
            String filePath = CTypeConversion.toJavaString(configFilePath);
            Map<String, Object> configs = loadConfigFromFile(filePath);
            
            // Initialize serializer and deserializer with the loaded configuration
            GlueSchemaRegistryConfiguration configuration = new GlueSchemaRegistryConfiguration(configs);
            SerializerInstance.create(configuration);
            DeserializerInstance.create(configuration);
            
            return 0; // Success
        } catch (Exception e) {
            System.err.println("Failed to load configuration file: " + e.getMessage());
            e.printStackTrace();
            return 1; // Error
        }
    }
    
    private static Map<String, Object> loadConfigFromFile(String filePath) throws IOException {
        Properties properties = new Properties();
        try (FileInputStream fis = new FileInputStream(filePath)) {
            properties.load(fis);
        }
        
        Map<String, Object> configs = new HashMap<>();
        for (String key : properties.stringPropertyNames()) {
            configs.put(key, properties.getProperty(key));
        }
        
        return configs;
    }
}