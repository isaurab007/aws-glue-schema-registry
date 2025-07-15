package com.amazonaws.services.schemaregistry;

import com.amazonaws.services.schemaregistry.common.Schema;
import com.amazonaws.services.schemaregistry.common.configs.GlueSchemaRegistryConfiguration;
import com.amazonaws.services.schemaregistry.serializer.ProtobufPreprocessor;
import com.amazonaws.services.schemaregistry.serializers.GlueSchemaRegistrySerializer;
import com.amazonaws.services.schemaregistry.utils.AWSSchemaRegistryConstants;
import com.google.common.collect.ImmutableMap;
import com.oracle.svm.core.c.CConst;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.CContext;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;
import org.graalvm.word.WordFactory;
import software.amazon.awssdk.services.glue.model.DataFormat;

import java.util.Map;
import java.util.HashMap;

import static com.amazonaws.services.schemaregistry.ByteArrayConverter.fromCReadOnlyByteArray;
import static com.amazonaws.services.schemaregistry.ByteArrayConverter.toCMutableByteArray;
import static com.amazonaws.services.schemaregistry.DataTypes.C_GlueSchemaRegistryErrorPointerHolder;
import static com.amazonaws.services.schemaregistry.DataTypes.C_GlueSchemaRegistrySchema;
import static com.amazonaws.services.schemaregistry.DataTypes.C_MutableByteArray;
import static com.amazonaws.services.schemaregistry.DataTypes.C_ReadOnlyByteArray;
import static com.amazonaws.services.schemaregistry.DataTypes.HandlerDirectives;

/**
 * Entry point class for the serialization methods of GSR shared library.
 */
@CContext(HandlerDirectives.class)
public class GlueSchemaRegistrySerializationHandler {

    @CEntryPoint(name = "initialize_serializer")
    public static void initializeSerializer(IsolateThread isolateThread) {
        // Default configuration for backward compatibility
        Map<String, String> configMap =
            ImmutableMap.of(
                AWSSchemaRegistryConstants.AWS_REGION,
                "us-east-1",
                AWSSchemaRegistryConstants.SCHEMA_AUTO_REGISTRATION_SETTING,
                "true"
            );
        GlueSchemaRegistryConfiguration glueSchemaRegistryConfiguration =
            new GlueSchemaRegistryConfiguration(configMap);

        SerializerInstance.create(glueSchemaRegistryConfiguration);
    }

    @CEntryPoint(name = "initialize_serializer_with_config")
    public static void initializeSerializerWithConfig(
        IsolateThread isolateThread,
        CCharPointer awsRegion,
        CCharPointer awsEndpoint,
        CCharPointer registryName,
        CCharPointer schemaName,
        int schemaAutoRegistration,
        CCharPointer compatibilitySetting,
        CCharPointer description,
        int cacheSize,
        long cacheTtlMillis,
        CCharPointer compressionType,
        CCharPointer secondaryDeserializer,
        CCharPointer dataFormat,
        CCharPointer protobufMessageType
    ) {
        // Convert C strings to Java and build configuration map
        Map<String, String> configMap = new HashMap<>();

        // AWS & Core Settings (P0)
        if (awsRegion.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.AWS_REGION, CTypeConversion.toJavaString(awsRegion));
        }
        if (awsEndpoint.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.AWS_ENDPOINT, CTypeConversion.toJavaString(awsEndpoint));
        }
        if (registryName.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.REGISTRY_NAME, CTypeConversion.toJavaString(registryName));
        }

        // Schema Management (P0)
        if (schemaName.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.SCHEMA_NAME, CTypeConversion.toJavaString(schemaName));
        }
        configMap.put(AWSSchemaRegistryConstants.SCHEMA_AUTO_REGISTRATION_SETTING, String.valueOf(schemaAutoRegistration == 1));
        if (compatibilitySetting.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.COMPATIBILITY_SETTING, CTypeConversion.toJavaString(compatibilitySetting));
        }
        if (description.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.DESCRIPTION, CTypeConversion.toJavaString(description));
        }

        // Performance & Caching (P0)
        configMap.put(AWSSchemaRegistryConstants.CACHE_SIZE, String.valueOf(cacheSize));
        configMap.put(AWSSchemaRegistryConstants.CACHE_TIME_TO_LIVE_MILLIS, String.valueOf(cacheTtlMillis));

        // Advanced Features (P0)
        if (compressionType.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.COMPRESSION_TYPE, CTypeConversion.toJavaString(compressionType));
        }
        if (secondaryDeserializer.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.SECONDARY_DESERIALIZER, CTypeConversion.toJavaString(secondaryDeserializer));
        }

        // Data Format Settings (P0)
        if (dataFormat.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.DATA_FORMAT, CTypeConversion.toJavaString(dataFormat));
        }
        if (protobufMessageType.isNonNull()) {
            configMap.put(AWSSchemaRegistryConstants.PROTOBUF_MESSAGE_TYPE, CTypeConversion.toJavaString(protobufMessageType));
        }
        
        // Create GSR configuration and initialize serializer
        GlueSchemaRegistryConfiguration glueSchemaRegistryConfiguration = new GlueSchemaRegistryConfiguration(configMap);
        SerializerInstance.create(glueSchemaRegistryConfiguration);
    }

    @CEntryPoint(name = "encode_with_schema")
    public static C_MutableByteArray encodeWithSchema(
        IsolateThread isolateThread,
        C_ReadOnlyByteArray c_readOnlyByteArray,
        @CConst CCharPointer c_transportName,
        C_GlueSchemaRegistrySchema c_glueSchemaRegistrySchema,
        C_GlueSchemaRegistryErrorPointerHolder errorPointerHolder) {
        try {

            //Access the input C schema object
            final String schemaName = CTypeConversion.toJavaString(c_glueSchemaRegistrySchema.getSchemaName());
            final String schemaDef;
            String rawSchema = CTypeConversion.toJavaString(c_glueSchemaRegistrySchema.getSchemaDef());
            final String dataFormat = CTypeConversion.toJavaString(c_glueSchemaRegistrySchema.getDataFormat());
            final String additionalSchemaInfor =
                CTypeConversion.toJavaString(c_glueSchemaRegistrySchema.getAdditionalSchemaInfo());
            final String transportName = CTypeConversion.toJavaString(c_transportName);

            //Because Protobuf is different from other data format drastically,
            //we will need to perform some pre-processing before precede
            if (DataFormat.PROTOBUF.name().equals(dataFormat)) {
                schemaDef = ProtobufPreprocessor.convertBase64SchemaToStringSchema(rawSchema);
            } else {
                schemaDef = rawSchema;
            }

            Schema javaSchema = new Schema(schemaDef, dataFormat, schemaName);

            //Read the c_byteArray data and create a new mutable byte array with encoded data
            byte[] bytesToEncode = fromCReadOnlyByteArray(c_readOnlyByteArray);

            if (DataFormat.PROTOBUF.name().equals(dataFormat)) {
                bytesToEncode =
                    ProtobufPreprocessor.prefixMessageIndexToBytes(bytesToEncode, schemaDef, additionalSchemaInfor);
            }

            //Assuming serializer instance is already initialized
            GlueSchemaRegistrySerializer glueSchemaRegistrySerializer = SerializerInstance.get();
            byte[] encodedBytes =
                glueSchemaRegistrySerializer.encode(transportName, javaSchema, bytesToEncode);

            return toCMutableByteArray(encodedBytes, errorPointerHolder);
        } catch (Exception | Error e) {

            ExceptionWriter.write(errorPointerHolder, e);

            return WordFactory.nullPointer();
        }
    }
}
