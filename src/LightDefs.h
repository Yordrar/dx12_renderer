#pragma once

#include <MathTypes.h>

struct PointLight
{
    float3 position;
    float3 color;
    float intensity;
};

struct DirectionalLight
{
    float3 direction;
    float3 color;
};

struct SpotLight
{
    float3 direction;
    float3 color;
    float angle;
};