#pragma once

#include "common.hlsli"

float normalDistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.1415926 * denom * denom;
	
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float geometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 calculateDirectLightingCookTorrance(float3 albedo, float3 normal, float roughness, float metallic, float3 viewVector, float3 lightVector, float3 lightColor, float shadow)
{
    float3 ambient = 0.03 * albedo;
    
    float3 H = normalize(viewVector + lightVector);
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    float3 F = fresnelSchlick(max(dot(H, viewVector), 0.0), F0);
    float NDF = normalDistributionGGX(normal, H, roughness);
    float G = geometrySmith(normal, viewVector, lightVector, roughness);
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * saturate(dot(normal, viewVector)) * saturate(dot(normal, lightVector)) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 diffuse = (1.0 - F) * albedo;
    
    float NdotL = saturate(dot(normal, lightVector));
    return ambient + shadow * ((diffuse + specular) * lightColor * NdotL);
}