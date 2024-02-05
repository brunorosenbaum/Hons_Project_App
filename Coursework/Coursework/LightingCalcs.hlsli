float4 calculateDirectionalLighting(float3 lightDirection, float3 normal, float4 ldiffuse, float4 ambient)
{
    float intensity = saturate(dot(normal, -lightDirection));
    float4 colour = saturate(ldiffuse * intensity) + ambient;
    return colour;
}

//Calculate attenuation depending on attenuation factors and distance. 
//float calcAttenuation(float constantFactor, float linearFactor, float quadraticFactor, float distance)
//{
//    return 1 / (constantFactor + (linearFactor * distance) + (quadraticFactor * pow(distance, 2)));
//}

float4 calculatePositionLighting(float3 worldPos, float3 lightPos, float3 normal, float4 diffuse, float4 ambient)
{
    //Includes attenuation calcs.
    //float attenuationFactors[3] = { 0.5f, 0.125f, 0.0f };
    float distance = length(lightPos - worldPos);
    //float attenuation = calcAttenuation(attenuationFactors[0], attenuationFactors[1], attenuationFactors[2], distance);

    //Light vector is just the light's position - the world's. 
    float3 lightVector = normalize(lightPos - worldPos);
    return calculateDirectionalLighting(-lightVector, normal, diffuse, ambient);
}