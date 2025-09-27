#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple test program to demonstrate GSR-like memory lifecycle for valgrind testing
// This simulates the init, encode, dispose pattern from the C# GSR implementation

typedef struct {
    char* registry_name;
    char* schema_name;
    void* encoded_data;
    size_t data_size;
} gsr_serializer_mock;

// Mock GSR serializer functions
gsr_serializer_mock* gsr_serializer_init(const char* registry_name, const char* schema_name) {
    printf("GSR Serializer: Initializing with registry='%s', schema='%s'\n", 
           registry_name ? registry_name : "default", 
           schema_name ? schema_name : "test-schema");
    
    gsr_serializer_mock* serializer = malloc(sizeof(gsr_serializer_mock));
    if (!serializer) {
        printf("GSR Serializer: Failed to allocate memory for serializer\n");
        return NULL;
    }
    
    // Allocate and copy registry name
    if (registry_name) {
        serializer->registry_name = malloc(strlen(registry_name) + 1);
        strcpy(serializer->registry_name, registry_name);
    } else {
        serializer->registry_name = malloc(8);
        strcpy(serializer->registry_name, "default");
    }
    
    // Allocate and copy schema name
    if (schema_name) {
        serializer->schema_name = malloc(strlen(schema_name) + 1);
        strcpy(serializer->schema_name, schema_name);
    } else {
        serializer->schema_name = malloc(12);
        strcpy(serializer->schema_name, "test-schema");
    }
    
    serializer->encoded_data = NULL;
    serializer->data_size = 0;
    
    printf("GSR Serializer: Initialization successful\n");
    return serializer;
}

int gsr_serializer_encode(gsr_serializer_mock* serializer, const char* data) {
    if (!serializer || !data) {
        printf("GSR Serializer: Invalid parameters for encode\n");
        return -1;
    }
    
    printf("GSR Serializer: Encoding data='%s'\n", data);
    
    // Free previous encoded data if exists
    if (serializer->encoded_data) {
        free(serializer->encoded_data);
    }
    
    // Simulate encoding by creating a prefixed version of the data
    const char* prefix = "GSR_ENCODED:";
    size_t prefix_len = strlen(prefix);
    size_t data_len = strlen(data);
    serializer->data_size = prefix_len + data_len + 1;
    
    serializer->encoded_data = malloc(serializer->data_size);
    if (!serializer->encoded_data) {
        printf("GSR Serializer: Failed to allocate memory for encoded data\n");
        serializer->data_size = 0;
        return -1;
    }
    
    strcpy((char*)serializer->encoded_data, prefix);
    strcat((char*)serializer->encoded_data, data);
    
    printf("GSR Serializer: Encoding successful, size=%zu\n", serializer->data_size);
    return 0;
}

void gsr_serializer_dispose(gsr_serializer_mock* serializer) {
    if (!serializer) {
        printf("GSR Serializer: Dispose called with NULL serializer\n");
        return;
    }
    
    printf("GSR Serializer: Disposing serializer\n");
    
    if (serializer->registry_name) {
        free(serializer->registry_name);
        serializer->registry_name = NULL;
    }
    
    if (serializer->schema_name) {
        free(serializer->schema_name);
        serializer->schema_name = NULL;
    }
    
    if (serializer->encoded_data) {
        free(serializer->encoded_data);
        serializer->encoded_data = NULL;
    }
    
    free(serializer);
    printf("GSR Serializer: Disposal complete\n");
}

// Test function that follows the C# GSR lifecycle pattern
int test_gsr_lifecycle_pattern() {
    printf("\n=== Testing GSR Serializer Lifecycle Pattern ===\n");
    
    // Test 1: Normal lifecycle
    printf("\n--- Test 1: Normal Lifecycle ---\n");
    gsr_serializer_mock* serializer = gsr_serializer_init("test-registry", "user-schema");
    if (!serializer) {
        printf("Test 1 FAILED: Could not initialize serializer\n");
        return 1;
    }
    
    if (gsr_serializer_encode(serializer, "test user data") != 0) {
        printf("Test 1 FAILED: Could not encode data\n");
        gsr_serializer_dispose(serializer);
        return 1;
    }
    
    printf("Test 1: Encoded data = '%s'\n", (char*)serializer->encoded_data);
    gsr_serializer_dispose(serializer);
    printf("Test 1 PASSED\n");
    
    // Test 2: Multiple encode operations (simulating reuse)
    printf("\n--- Test 2: Multiple Encode Operations ---\n");
    serializer = gsr_serializer_init("prod-registry", "order-schema");
    if (!serializer) {
        printf("Test 2 FAILED: Could not initialize serializer\n");
        return 1;
    }
    
    // First encode
    if (gsr_serializer_encode(serializer, "order-1") != 0) {
        printf("Test 2 FAILED: Could not encode first data\n");
        gsr_serializer_dispose(serializer);
        return 1;
    }
    printf("Test 2: First encoded data = '%s'\n", (char*)serializer->encoded_data);
    
    // Second encode (should replace first)
    if (gsr_serializer_encode(serializer, "order-2") != 0) {
        printf("Test 2 FAILED: Could not encode second data\n");
        gsr_serializer_dispose(serializer);
        return 1;
    }
    printf("Test 2: Second encoded data = '%s'\n", (char*)serializer->encoded_data);
    
    gsr_serializer_dispose(serializer);
    printf("Test 2 PASSED\n");
    
    // Test 3: Error handling
    printf("\n--- Test 3: Error Handling ---\n");
    if (gsr_serializer_encode(NULL, "test") == 0) {
        printf("Test 3 FAILED: Should have failed with NULL serializer\n");
        return 1;
    }
    
    serializer = gsr_serializer_init("error-registry", "error-schema");
    if (gsr_serializer_encode(serializer, NULL) == 0) {
        printf("Test 3 FAILED: Should have failed with NULL data\n");
        gsr_serializer_dispose(serializer);
        return 1;
    }
    
    gsr_serializer_dispose(serializer);
    printf("Test 3 PASSED\n");
    
    printf("\n=== All GSR Lifecycle Tests Passed ===\n");
    return 0;
}

int main() {
    printf("GSR Serializer Memory Lifecycle Test for Valgrind\n");
    printf("This test simulates the init, encode, dispose pattern from C# GSR implementation\n");
    
    int result = test_gsr_lifecycle_pattern();
    
    if (result == 0) {
        printf("\nSUCCESS: All tests passed - ready for valgrind analysis\n");
    } else {
        printf("\nFAILED: Some tests failed\n");
    }
    
    return result;
}
