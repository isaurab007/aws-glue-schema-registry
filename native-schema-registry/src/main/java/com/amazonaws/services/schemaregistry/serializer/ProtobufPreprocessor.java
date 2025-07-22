package com.amazonaws.services.schemaregistry.serializer;

import com.amazonaws.services.schemaregistry.serializers.protobuf.MessageIndexFinder;
import com.amazonaws.services.schemaregistry.serializers.protobuf.ProtobufWireFormatEncoder;
import com.amazonaws.services.schemaregistry.deserializers.protobuf.ProtobufSchemaParser;
import com.amazonaws.services.schemaregistry.utils.apicurio.FileDescriptorUtils;
import com.google.common.collect.BiMap;
import com.google.protobuf.DescriptorProtos;
import com.google.protobuf.Descriptors;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.CodedOutputStream;

import java.io.IOException;
import java.io.ByteArrayOutputStream;
import java.util.Base64;

public class ProtobufPreprocessor {
    private static final ProtobufWireFormatEncoder ENCODER = new ProtobufWireFormatEncoder(new MessageIndexFinder());
    private static final MessageIndexFinder MESSAGE_INDEX_FINDER = new MessageIndexFinder();

    public static String convertBase64SchemaToStringSchema(String base64Schema) throws InvalidProtocolBufferException {
        DescriptorProtos.FileDescriptorProto
            fileDescriptorProto =
            DescriptorProtos.FileDescriptorProto.parseFrom(Base64.getDecoder().decode(base64Schema));

        // Use FileDescriptorUtils.fileDescriptorToProtoFile() as replacement for the missing method
        String rawSchema = FileDescriptorUtils.fileDescriptorToProtoFile(fileDescriptorProto).toSchema();
        // Remove the header that Wire adds, similar to the original implementation
        String headerToRemove = "// Proto schema formatted by Wire, do not edit.\n// Source: \n\n";
        return rawSchema.replace(headerToRemove, "");
    }

    public static byte[] prefixMessageIndexToBytes(byte[] bytesToEncode, String schemaDef, String descriptorFullname)
        throws Descriptors.DescriptorValidationException, IOException, NoSuchMethodException {
        
        // Parse the schema to get the FileDescriptor
        Descriptors.FileDescriptor fileDescriptor = ProtobufSchemaParser.parse(schemaDef, "any-name.proto");
        
        // Find the specific descriptor by iterating through message types
        Descriptors.Descriptor targetDescriptor = findDescriptorByFullName(fileDescriptor, descriptorFullname);
        
        if (targetDescriptor == null) {
            // If descriptor not found, return original bytes
            return bytesToEncode;
        }
        
        // Use the public API to get the message index
        Integer messageIndex = MESSAGE_INDEX_FINDER.getByDescriptor(fileDescriptor, targetDescriptor);
        
        // Manually prefix the message index to the bytes
        return prefixMessageIndexManually(bytesToEncode, messageIndex);
    }
    
    /**
     * Helper method to find a descriptor by its full name within a FileDescriptor
     */
    private static Descriptors.Descriptor findDescriptorByFullName(Descriptors.FileDescriptor fileDescriptor, String fullName) {
        // Search through all message types in the file descriptor
        for (Descriptors.Descriptor messageType : fileDescriptor.getMessageTypes()) {
            Descriptors.Descriptor found = searchDescriptorRecursively(messageType, fullName);
            if (found != null) {
                return found;
            }
        }
        return null;
    }
    
    /**
     * Recursively search for a descriptor by full name, including nested types
     */
    private static Descriptors.Descriptor searchDescriptorRecursively(Descriptors.Descriptor descriptor, String fullName) {
        if (descriptor.getFullName().equals(fullName)) {
            return descriptor;
        }
        
        // Search nested types
        for (Descriptors.Descriptor nestedType : descriptor.getNestedTypes()) {
            Descriptors.Descriptor found = searchDescriptorRecursively(nestedType, fullName);
            if (found != null) {
                return found;
            }
        }
        
        return null;
    }
    
    /**
     * Manually prefix the message index to the byte array
     * This replicates what ProtobufWireFormatEncoder.encode() does internally
     */
    private static byte[] prefixMessageIndexManually(byte[] originalBytes, Integer messageIndex) throws IOException {
        java.io.ByteArrayOutputStream outputStream = new java.io.ByteArrayOutputStream();
        com.google.protobuf.CodedOutputStream codedOutputStream = com.google.protobuf.CodedOutputStream.newInstance(outputStream);
        
        try {
            // Write the messageIndex as variable sized int (same as encoder does)
            codedOutputStream.writeUInt32NoTag(messageIndex);
            
            // Write the original protobuf message bytes
            codedOutputStream.writeRawBytes(originalBytes);
            codedOutputStream.flush();
            
        } catch (IOException e) {
            throw e;
        }
        
        return outputStream.toByteArray();
    }
}
