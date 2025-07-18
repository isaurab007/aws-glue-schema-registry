# Configuration File Feature for AWS Glue Schema Registry

This document describes how to use the configuration file feature in the AWS Glue Schema Registry native stack.

## Overview

The configuration file feature allows you to specify a properties file that contains configuration settings for the AWS Glue Schema Registry. This file is passed from the C# layer down through the C layer to the Java layer, where the settings are applied to the schema registry operations.

## Configuration File Format

The configuration file uses the Java Properties format:

```properties
# AWS Glue Schema Registry Configuration
aws.region=us-east-1
registry.name=my-registry
schema.auto.registration=true
cache.time.to.live.millis=60000
cache.size=100
```

## Usage in C#

```csharp
using AWSGsrSerDe;

// Set the configuration file path
GlueSchemaRegistryConfig.SetConfigurationFile("/path/to/config.properties");

// Now create serializers/deserializers as usual
var serializer = new GlueSchemaRegistrySerializer();
var deserializer = new GlueSchemaRegistryDeserializer();
```

## Supported Configuration Properties

The following properties are supported in the configuration file:

| Property | Description | Default Value |
|----------|-------------|---------------|
| aws.region | AWS Region | Default from AWS SDK |
| registry.name | Schema Registry name | default-registry |
| schema.auto.registration | Enable auto-registration of schemas | false |
| cache.time.to.live.millis | Cache TTL in milliseconds | 24 hours |
| cache.size | Maximum number of elements in cache | 200 |

## Implementation Details

The configuration file path is passed from:
1. C# layer (`GlueSchemaRegistryConfig.SetConfigurationFile`)
2. Through the C layer (`glue_schema_registry_set_config_file`)
3. To the Java layer (`ConfigurationFileReader.readConfigurationFile`)

The Java layer reads the properties file and applies the settings to both the serializer and deserializer instances.

## Error Handling

If the configuration file does not exist or cannot be read, an appropriate exception will be thrown:
- In C#: `FileNotFoundException` or `AwsSchemaRegistryException`
- In C: Error is set in the `p_err` parameter
- In Java: Exception is logged and a non-zero status code is returned