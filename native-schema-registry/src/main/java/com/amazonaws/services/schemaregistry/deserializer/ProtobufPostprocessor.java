package com.amazonaws.services.schemaregistry.deserializer;

import com.google.protobuf.CodedInputStream;
import org.apache.commons.lang3.ArrayUtils;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class ProtobufPostprocessor {
    public static byte[] stripMessageIndex(byte[] data) throws IOException {
        // Create CodedInputStream from the data
        CodedInputStream inputStream = CodedInputStream.newInstance(data);
        
        // Read and discard the message index (UInt32)
        inputStream.readUInt32();
        
        // Read the remaining bytes (the actual protobuf message)
        List<Byte> output = new ArrayList<>();
        while (!inputStream.isAtEnd()) {
            byte b = inputStream.readRawByte();
            output.add(b);
        }
        Byte[] result = new Byte[output.size()];
        return ArrayUtils.toPrimitive(output.toArray(result));
    }
}
