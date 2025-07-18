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

namespace AWSGsrSerDe.deserializer
{
    /// <summary>
    /// Extension methods for GlueSchemaRegistryKafkaDeserializer.
    /// </summary>
    public static class GlueSchemaRegistryKafkaDeserializerExtensions
    {
        /// <summary>
        /// Creates a new instance of GlueSchemaRegistryKafkaDeserializer with configuration from a YAML file.
        /// </summary>
        /// <param name="configFilePath">Path to the YAML configuration file</param>
        /// <returns>A new instance of GlueSchemaRegistryKafkaDeserializer</returns>
        public static GlueSchemaRegistryKafkaDeserializer FromConfigFile(string configFilePath)
        {
            var config = ConfigurationFileReader.ReadConfigurationFile(configFilePath);
            return new GlueSchemaRegistryKafkaDeserializer(config);
        }
        
        /// <summary>
        /// Configures an existing GlueSchemaRegistryKafkaDeserializer with configuration from a YAML file.
        /// </summary>
        /// <param name="deserializer">The deserializer to configure</param>
        /// <param name="configFilePath">Path to the YAML configuration file</param>
        public static void ConfigureFromFile(this GlueSchemaRegistryKafkaDeserializer deserializer, string configFilePath)
        {
            var config = ConfigurationFileReader.ReadConfigurationFile(configFilePath);
            deserializer.Configure(config);
        }
    }
}