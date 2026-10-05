struct VSInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer SpriteConstants : register(b0)
{
    float2 spriteCenter;
    float2 spriteSize;
    float2 screenSize;
    float spriteRotationRadians;
    float spriteTransformPadding;
};

float2 RotateLocalPosition(float2 localPosition, float angleRadians)
{
    float2 rotatedXAxis =
        float2(cos(angleRadians), sin(angleRadians));

    float2 rotatedYAxis =
        float2(-sin(angleRadians), cos(angleRadians));

    float2 r1 = rotatedXAxis * localPosition.x;
    float2 r2 = rotatedYAxis * localPosition.y;

    return r1 + r2;
}

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float2 localPosition = float2(input.position.x, -input.position.y);
    float2 rotatedPosition = RotateLocalPosition(localPosition, spriteRotationRadians);
    float2 pixelPosition = spriteCenter + rotatedPosition * spriteSize;
    float2 ndcPosition = float2(
        2.0f * pixelPosition.x / screenSize.x - 1.0f,
        1.0f - 2.0f * pixelPosition.y / screenSize.y);
    output.position = float4(ndcPosition, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}

Texture2D spriteTexture : register(t0);
SamplerState spriteSampler : register(s0);

cbuffer SpriteTintConstants : register(b1)
{
    float4 spriteTint;
    float spriteTimeSource;
    float spriteShape;
    float2 spriteTintPadding;
    float4 spriteUVRect;
    float dissolveProgress;
    float dissolveNoiseScale;
    float dissolveEdgeWidth;
    float dissolveEdgeStrength;
    float4 dissolveEdgeColor;
};

cbuffer FrameConstants : register(b2)
{
    float gameTimeSeconds;
    float realTimeSeconds;
    float2 framePadding;
};

static const float kPulsePeriodSeconds = 2.0f;
static const float kTwoPi = 6.28318530718f;

float ComputePulse(float timeSeconds)
{
    float u = 0.5 + 0.5 * sin(kTwoPi * timeSeconds / kPulsePeriodSeconds);
    float a = 0.25;
    float b = 1.0;
    return a + (b - a) * u;
}

float2 TransformSpriteUV(float2 uv)
{
    float u = lerp(spriteUVRect.xy.x, spriteUVRect.zw.x, uv.x);
    float v = lerp(spriteUVRect.xy.y, spriteUVRect.zw.y, uv.y);
    return float2(u,v);
}

static const float kShapeInnerRadius = 0.3f;
static const float kShapeOuterRadius = 0.5f;

float ComputeShapeAlpha(float2 localUV)
{
    float v = length(localUV - float2(0.5, 0.5));
    return 1 - smoothstep(kShapeInnerRadius, kShapeOuterRadius, v);
}

static const float kGlowRadius = 0.5f;
static const float kGlowFalloffExponent = 2.0f;

float ComputeGlowIntensity(float2 localUV)
{
    float distance = length(localUV - float2(0.5, 0.5));
    float normalizedDistance = distance / kGlowRadius;

    return pow(saturate(1 - normalizedDistance), kGlowFalloffExponent);
}

float HashDissolveCell(float2 cell)
{
    return frac(sin(dot(cell, float2(127.1f, 311.7f))) * 43758.5453f);
}

float ComputeDissolveNoise(float2 localUV)
{
    float2 position = localUV * max(dissolveNoiseScale, 1.0f);
    float2 cell = floor(position);
    float2 fraction = frac(position);
    float2 blend = fraction * fraction * (3.0f - 2.0f * fraction);
    float a = HashDissolveCell(cell);
    float b = HashDissolveCell(cell + float2(1.0f, 0.0f));
    float c = HashDissolveCell(cell + float2(0.0f, 1.0f));
    float d = HashDissolveCell(cell + float2(1.0f, 1.0f));
    return lerp(lerp(a, b, blend.x), lerp(c, d, blend.x), blend.y);
}

void ApplyDissolve(float noiseValue, float progress)
{
    if (progress <= 0.0f)
        return;
    if (progress >= 1.0f)
    {
        clip(-1.0f);
        return;
    }
    if(noiseValue < progress)
    {
        clip(-1.0f);
        return;
    }
    return;
}

float ComputeDissolveEdgeIntensity(float noiseValue, float progress, float edgeWidth)
{
    if (edgeWidth <= 0.0f)
        return 0.0f;

    float t = noiseValue - progress;

    if(t >= 0 && t <= edgeWidth) {
        return saturate(1 - t / edgeWidth);
    }

    return 0.0f;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float2 sampleUV = TransformSpriteUV(input.uv);
    float4 color = spriteTexture.Sample(spriteSampler, sampleUV) * spriteTint;
    if (dissolveProgress > 0.0f)
    {
        float noiseValue = ComputeDissolveNoise(input.uv);
        ApplyDissolve(noiseValue, dissolveProgress);
        float edgeIntensity = ComputeDissolveEdgeIntensity(
            noiseValue, dissolveProgress, dissolveEdgeWidth);
        color.rgb += dissolveEdgeColor.rgb * dissolveEdgeStrength * edgeIntensity;
    }
    float effectTime = spriteTimeSource > 0.5f ? realTimeSeconds : gameTimeSeconds;
    //color.rgb *= ComputePulse(effectTime);
    if (spriteShape > 1.5f)
    {
        color.a *= ComputeGlowIntensity(input.uv);
    }
    else if (spriteShape > 0.5f)
    {
        color.a *= ComputeShapeAlpha(input.uv);
    }
    return color;
}
