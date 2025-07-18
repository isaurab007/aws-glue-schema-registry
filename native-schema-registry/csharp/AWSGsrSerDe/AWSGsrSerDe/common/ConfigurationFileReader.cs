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

using System;
using System.Collections.Generic;
using System.IO;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

namespace AWSGsrSerDe.common
{
    /// <summary>
    /// Reads configuration from a YAML file and converts it to a dictionary for use with the schema registry.
    /// </summary>
    public static class ConfigurationFileReader
    {
        /// <summary>
        /// Reads configuration from a YAML file.
        /// </summary>
        /// <param name="configFilePath">Path to the YAML configuration file</param>
        /// <returns>Dictionary containing the configuration values</returns>
        /// <exception cref="FileNotFoundException">Thrown when the configuration file does not exist</exception>
        /// <exception cref="Exception">Thrown when there is an error reading the configuration file</exception>
        public static Dictionary<string, dynamic> ReadConfigurationFile(string configFilePath)
        {
            if (!File.Exists(configFilePath))
            {
                throw new FileNotFoundException("Configuration file not found", configFilePath);
            }

            try
            {
                var deserializer = new DeserializerBuilder()
                    .WithNamingConvention(CamelCaseNamingConvention.Instance)
                    .Build();

                var yamlText = File.ReadAllText(configFilePath);
                var yamlObject = deserializer.Deserialize<Dictionary<object, object>>(yamlText);
                
                // Convert to the format expected by GlueSchemaRegistryConfiguration
                var config = new Dictionary<string, dynamic>();
                
                foreach (var entry in yamlObject)
                {
                    var key = entry.Key.ToString();
                    var value = entry.Value;
                    
                    // Map YAML keys to configuration constants
                    switch (key)
                    {
                        case "data.format":
                            if (Enum.TryParse<GlueSchemaRegistryConstants.DataFormat>(value.ToString(), out var dataFormat))
                            {
                                config[GlueSchemaRegistryConstants.DataFormatType] = dataFormat;
                            }
                            break;
                        case "avro.record.type":
                            if (Enum.TryParse<AvroRecordType>(value.ToString(), out var avroRecordType))
                            {
                                config[GlueSchemaRegistryConstants.AvroRecordType] = avroRecordType;
                            }
                            break;
                        // Add other mappings as needed
                        default:
                            // Store other values directly
                            config[key] = value;
                            break;
                    }
                }
                
                return config;
            }
            catch (Exception ex)
            {
                throw new Exception($"Error reading configuration file: {ex.Message}", ex);
            }
        }
    }
}