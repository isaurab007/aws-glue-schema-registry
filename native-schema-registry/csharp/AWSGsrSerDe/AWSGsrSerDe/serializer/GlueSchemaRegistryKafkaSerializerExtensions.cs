// Copyright 2023 Amazon.com, Inc. or its affiliates.
// Licensed under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//  
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using System.Collections.Generic;
using AWSGsrSerDe.common;

namespace AWSGsrSerDe.serializer
{
    /// <summary>
    /// Extension methods for GlueSchemaRegistryKafkaSerializer.
    /// </summary>
    public static class GlueSchemaRegistryKafkaSerializerExtensions
    {
        /// <summary>
        /// Creates a new instance of GlueSchemaRegistryKafkaSerializer with configuration from a YAML file.
        /// </summary>
        /// <param name="configFilePath">Path to the YAML configuration file</param>
        /// <returns>A new instance of GlueSchemaRegistryKafkaSerializer</returns>
        public static GlueSchemaRegistryKafkaSerializer FromConfigFile(string configFilePath)
        {
            var config = ConfigurationFileReader.ReadConfigurationFile(configFilePath);
            return new GlueSchemaRegistryKafkaSerializer(config);
        }
        
        /// <summary>
        /// Configures an existing GlueSchemaRegistryKafkaSerializer with configuration from a YAML file.
        /// </summary>
        /// <param name="serializer">The serializer to configure</param>
        /// <param name="configFilePath">Path to the YAML configuration file</param>
        public static void ConfigureFromFile(this GlueSchemaRegistryKafkaSerializer serializer, string configFilePath)
        {
            var config = ConfigurationFileReader.ReadConfigurationFile(configFilePath);
            serializer.Configure(config);
        }
    }
}