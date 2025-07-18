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
using System.IO;

namespace AWSGsrSerDe
{
    /// <summary>
    /// Provides configuration functionality for the AWS Glue Schema Registry.
    /// </summary>
    public class GlueSchemaRegistryConfig
    {
        /// <summary>
        /// Sets the configuration file path for the schema registry.
        /// </summary>
        /// <param name="configFilePath">Path to the configuration file</param>
        /// <exception cref="FileNotFoundException">Thrown when the configuration file does not exist</exception>
        /// <exception cref="AwsSchemaRegistryException">Thrown when there is an error setting the configuration</exception>
        public static void SetConfigurationFile(string configFilePath)
        {
            if (!File.Exists(configFilePath))
            {
                throw new FileNotFoundException("Configuration file not found", configFilePath);
            }

            SWIGTYPE_p_p_glue_schema_registry_error p_err = GsrSerDePINVOKE.new_glue_schema_registry_error_ptr();
            int result = GsrSerDePINVOKE.glue_schema_registry_set_config_file(configFilePath, p_err);
            
            if (result != 0)
            {
                throw new AwsSchemaRegistryException("Failed to set configuration file");
            }
        }
    }
}