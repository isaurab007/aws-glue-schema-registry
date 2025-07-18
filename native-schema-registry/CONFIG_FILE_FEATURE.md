# Configuration File Feature for AWS Glue Schema Registry

This document describes how to use the configuration file feature in the AWS Glue Schema Registry native stack.

## Overview

The configuration file feature allows you to specify a YAML file that contains configuration settings for the AWS Glue Schema Registry. This simplifies the process of configuring the serializer and deserializer by keeping all settings in a single file.

## Configuration File Format

The configuration file uses YAML format:

```yaml
# AWS Glue Schema Registry Configuration

# AWS Region
aws.region: us-east-1

# Schema Registry settings
registry.name: my-registry
schema.auto.registration: true

# Cache settings
cache.time.to.live.millis: 60000
cache.size: 100

# Data format settings
data.format: AVRO
avro.record.type: GENERIC_RECORD

# Compression settings
compression.type: ZLIB
```

## Usage in C#

### Creating a serializer from a configuration file

```csharp
using AWSGsrSerDe.serializer;

// Create a serializer from a configuration file
var serializer = GlueSchemaRegistryKafkaSerializerExtensions.FromConfigFile("/path/to/config.yaml");

// Use the serializer
byte[] serializedData = serializer.Serialize(data, topic);
```

### Configuring an existing serializer from a configuration file

```csharp
using AWSGsrSerDe.serializer;

// Create a serializer
var serializer = new GlueSchemaRegistryKafkaSerializer(new Dictionary<string, dynamic>());

// Configure it from a file
serializer.ConfigureFromFile("/path/to/config.yaml");

// Use the serializer
byte[] serializedData = serializer.Serialize(data, topic);
```

### Creating a deserializer from a configuration file

```csharp
using AWSGsrSerDe.deserializer;

// Create a deserializer from a configuration file
var deserializer = GlueSchemaRegistryKafkaDeserializerExtensions.FromConfigFile("/path/to/config.yaml");

// Use the deserializer
object deserializedData = deserializer.Deserialize(serializedData, topic);
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
| data.format | Data format (AVRO, JSON, PROTOBUF) | - |
| avro.record.type | Avro record type (GENERIC_RECORD, SPECIFIC_RECORD) | - |
| compression.type | Compression type (ZLIB, NONE) | NONE |

## Implementation Details

The configuration file is read by the C# layer and converted to a dictionary that is passed to the serializer and deserializer. The configuration is then passed through the existing mechanisms to the Java layer.

## Error Handling

If the configuration file does not exist or cannot be read, an appropriate exception will be thrown:
- `FileNotFoundException` if the file does not exist
- `Exception` with details if there is an error reading or parsing the file