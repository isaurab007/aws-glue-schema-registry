package com.amazonaws.services.schemaregistry;

import com.amazonaws.services.schemaregistry.common.Schema;
import com.amazonaws.services.schemaregistry.common.configs.GlueSchemaRegistryConfiguration;
import com.amazonaws.services.schemaregistry.deserializer.ProtobufPostprocessor;
import com.amazonaws.services.schemaregistry.deserializers.GlueSchemaRegistryDeserializer;
import com.amazonaws.services.schemaregistry.utils.AWSSchemaRegistryConstants;
import com.google.common.collect.ImmutableMap;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.CContext;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;
import org.graalvm.word.WordFactory;
import software.amazon.awssdk.services.glue.model.DataFormat;

import java.util.HashMap;
import java.util.Map;

import static com.amazonaws.services.schemaregistry.ByteArrayConverter.fromCReadOnlyByteArray;
import static com.amazonaws.services.schemaregistry.ByteArrayConverter.toCMutableByteArray;
import static com.amazonaws.services.schemaregistry.DataTypes.C_GlueSchemaRegistryErrorPointerHolder;
import static com.amazonaws.services.schemaregistry.DataTypes.C_GlueSchemaRegistrySchema;
import static com.amazonaws.services.schemaregistry.DataTypes.C_MutableByteArray;
import static com.amazonaws.services.schemaregistry.DataTypes.C_ReadOnlyByteArray;
import static com.amazonaws.services.schemaregistry.DataTypes.HandlerDirectives;
import static com.amazonaws.services.schemaregistry.DataTypes.newGlueSchemaRegistrySchema;

/**
 * Entry point class for the serialization methods of GSR shared library.
 */
@CContext(HandlerDirectives.class)
public class GlueSchemaRegistryDeserializationHandler {

    @CEntryPoint(name = "initialize_deserializer")
    public static void initializeDeserializer(IsolateThread isolateThread) {
        // Default configuration for backward compatibility
        Map<String, String> configMap =
            ImmutableMap.of(
                AWSSchemaRegistryConstants.AWS_REGION,
                "us-east-1"
            );
        GlueSchemaRegistryConfiguration glueSchemaRegistryConfiguration =
            new GlueSchemaRegistryConfiguration(configMap);

        DeserializerInstance.create(glueSchemaRegistryConfiguration);
    }
    
    @CEntryPoint(name = "initialize_deserializer_with_config")
    public static void initializeDeserializerWithConfig(
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
        
        // Create GSR configuration and initialize deserializer
        GlueSchemaRegistryConfiguration glueSchemaRegistryConfiguration = new GlueSchemaRegistryConfiguration(configMap);
        DeserializerInstance.create(glueSchemaRegistryConfiguration);
    }

    @CEntryPoint(name = "decode")
    public static C_MutableByteArray decode(
        IsolateThread isolateThread,
        C_ReadOnlyByteArray c_readOnlyByteArray,
        C_GlueSchemaRegistryErrorPointerHolder errorPointer) {

        try {
            byte[] bytesToDecode = fromCReadOnlyByteArray(c_readOnlyByteArray);

            //Assuming deserializer instance is already initialized
            GlueSchemaRegistryDeserializer glueSchemaRegistryDeserializer = DeserializerInstance.get();

            byte[] decodedBytes =
                glueSchemaRegistryDeserializer.getData(bytesToDecode);

            //Get the schema and perform Protobuf post-processing if needed
            Schema decodedSchema =
                glueSchemaRegistryDeserializer.getSchema(bytesToDecode);
            if (DataFormat.PROTOBUF.name().equals(decodedSchema.getDataFormat())) {
                decodedBytes = ProtobufPostprocessor.stripMessageIndex(decodedBytes);
            }

            return toCMutableByteArray(decodedBytes, errorPointer);
        } catch (Exception | Error e) {
            ExceptionWriter.write(errorPointer, e);
            return WordFactory.nullPointer();
        }
    }

    @CEntryPoint(name = "decode_schema")
    public static C_GlueSchemaRegistrySchema decodeSchema(
        IsolateThread isolateThread,
        C_ReadOnlyByteArray c_readOnlyByteArray,
        C_GlueSchemaRegistryErrorPointerHolder errorPointer) {

        try {
            byte[] bytesToDecode = fromCReadOnlyByteArray(c_readOnlyByteArray);

            //Assuming serializer instance is already initialized
            GlueSchemaRegistryDeserializer glueSchemaRegistryDeserializer = DeserializerInstance.get();
            Schema decodedSchema =
                glueSchemaRegistryDeserializer.getSchema(bytesToDecode);

            CTypeConversion.CCharPointerHolder cSchemaNamePointer =
                CTypeConversion.toCString(decodedSchema.getSchemaName());
            CTypeConversion.CCharPointerHolder cSchemaDefPointer =
                CTypeConversion.toCString(decodedSchema.getSchemaDefinition());
            CTypeConversion.CCharPointerHolder cDataFormatPointer =
                CTypeConversion.toCString(decodedSchema.getDataFormat());

            //TODO: We can potentially expose the C Strings to target language layer to
            //prevent copying strings repeatedly.
            C_GlueSchemaRegistrySchema c_glueSchemaRegistrySchema = newGlueSchemaRegistrySchema(
                cSchemaNamePointer.get(),
                cSchemaDefPointer.get(),
                cDataFormatPointer.get(),
                errorPointer
            );
            //newGlueSchemaRegistrySchema has it's own copy of these attributes.
            cDataFormatPointer.close();
            cSchemaDefPointer.close();
            cSchemaNamePointer.close();

            return c_glueSchemaRegistrySchema;
        } catch (Exception | Error e) {
            ExceptionWriter.write(errorPointer, e);
            return WordFactory.nullPointer();
        }
    }

    @CEntryPoint(name = "can_decode")
    public static byte canDecode(
        IsolateThread isolateThread,
        C_ReadOnlyByteArray c_readOnlyByteArray,
        C_GlueSchemaRegistryErrorPointerHolder errorPointer) {
        try {

            byte[] bytesToDecode = fromCReadOnlyByteArray(c_readOnlyByteArray);

            GlueSchemaRegistryDeserializer glueSchemaRegistryDeserializer = DeserializerInstance.get();
            boolean canDeserialize =
                glueSchemaRegistryDeserializer.canDeserialize(bytesToDecode);

            if (errorPointer.isNonNull()) {
                errorPointer.write(WordFactory.nullPointer());
            }
            return CTypeConversion.toCBoolean(canDeserialize);
        } catch (Exception | Error e) {
            ExceptionWriter.write(errorPointer, e);
            return CTypeConversion.toCBoolean(false);
        }
    }
}
