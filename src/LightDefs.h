#pragma once

#include <core/Math.h>

struct PointLight
{
    float4 position;
    float4 color;
    float intensity;
};

struct DirectionalLight
{
    float4 direction;
    float4 color;
};

struct SpotLight
{
    float4 direction;
    float4 color;
    float angle;
};