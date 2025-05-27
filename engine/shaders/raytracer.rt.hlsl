#include "math.hlsl"

RaytracingAccelerationStructure SceneTlas : register(t0);
RWTexture2D<float4> OutRenderTarget : register(u0);



//struct Vertex
//{
//    float3 Position;
//    float3 Normal;
//};
//
//StructuredBuffer<float3> VertexPositions : register(t1);
//StructuredBuffer<float3> VertexNormals : register(t2);

struct Payload
{
    float3 Colour;
    bool AllowReflection; // #Todo: optimize this - can be turned into bitwise flags.
    bool Missed;
};

// #Todo: All values defined below before shader logic should be added into a constant buffer based on scene/camera descriptions.
static const float CameraFovInDeg = 70;
static const float CameraFocalLength = 2;
static const float4x4 CameraWorldTransform = float4x4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 1.5, -7, 1
);

static const float3 LightPosition = float3(0, 200, 0);
static const float3 SkyTopColour = float3(0.24, 0.44, 0.72);
static const float3 SkyBottomColour = float3(0.75, 0.86, 0.93);

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 pixelIdx = DispatchRaysIndex().xy;
    const float2 pixelTotal = DispatchRaysDimensions().xy;

    const float imagePlaneScale = tan(DegToRad(CameraFovInDeg) / 2.0);
    const float2 imagePlaneDims = float2(2.0f * imagePlaneScale * (pixelTotal.x / pixelTotal.y), -2.0f * imagePlaneScale);
    const float2 imagePlaneOffset = -0.5f * imagePlaneDims;
    const float2 pixelDelta = imagePlaneDims / pixelTotal;
    
    const float3 cameraPosition = float3(CameraWorldTransform[3].xyz);
    const float4 rayDirection = mul(CameraWorldTransform, float4(
        imagePlaneOffset.x + pixelIdx.x * pixelDelta.x,
        imagePlaneOffset.y + pixelIdx.y * pixelDelta.y,
        CameraFocalLength,
        0
    ));

    const RayDesc rayDesc =
    {
        cameraPosition,
		0.001,
		rayDirection.xyz,
		1000
    };

    Payload rayPayload =
    {
        float3(0.0, 0.0, 0.0),
		true,
		false
    };

    TraceRay(SceneTlas, RAY_FLAG_NONE, 0xFF, 0, 0, 0, rayDesc, rayPayload);

    OutRenderTarget[pixelIdx] = float4(rayPayload.Colour, 1);
}

[shader("miss")]
void Miss(inout Payload a_payload)
{
    const float slope = normalize(WorldRayDirection()).y;
    const float t = saturate(slope * 5 + 0.5);
    a_payload.Colour = lerp(SkyBottomColour, SkyTopColour, t);
    a_payload.Missed = true;
}

void HitMesh(inout Payload a_payload, float2 a_uv);
void HitMirror(inout Payload a_payload, float2 a_uv);
void HitFloor(inout Payload a_payload, float2 a_uv);

[shader("closesthit")]
void ClosestHit(inout Payload a_payload, BuiltInTriangleIntersectionAttributes a_attrib)
{
    switch (InstanceID())
    {
    case 0:
        HitMesh(a_payload, a_attrib.barycentrics);
        break;
    case 1:
        HitMirror(a_payload, a_attrib.barycentrics);
        break;
    case 2:
        HitFloor(a_payload, a_attrib.barycentrics);
        break;
    default:
        a_payload.Colour = float3(1, 0, 1);
        break;
    }
}

void HitMesh(inout Payload a_payload, float2 a_uv)
{
    const float depthScale = 1.0 / RayTCurrent();
    a_payload.Colour = float3(depthScale, depthScale, depthScale);
}

void HitMirror(inout Payload a_payload, float2 a_uv)
{
    if (!a_payload.AllowReflection)
    {
        return;
    }

    const float3 rayHitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    const float3 worldNormal = normalize(mul(float3(0, 1, 0), (float3x3) ObjectToWorld4x3()));
    const float3 reflectedWorldDirection = reflect(normalize(WorldRayDirection()), worldNormal);

    const RayDesc mirrorRayDesc =
    {
        rayHitPosition,
		0.001,
		reflectedWorldDirection,
		1000
    };

    a_payload.AllowReflection = false;
    TraceRay(SceneTlas, RAY_FLAG_NONE, 0xFF, 0, 0, 0, mirrorRayDesc, a_payload);
}

void HitFloor(inout Payload a_payload, float2 a_uv)
{
    const float3 rayHitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    const bool2 checkerPattern = frac(rayHitPosition.xz) > 0.5;
    a_payload.Colour = checkerPattern.x ^ checkerPattern.y ? 0.6.xxx : 0.4.xxx;
	
    const RayDesc shadowRayDesc =
    {
        rayHitPosition,
		0.001,
		LightPosition - rayHitPosition,
		1
    };
    
    Payload shadowPayload =
    {
        float3(0.0, 0.0, 0.0),
		false,
		false
    };
    
    TraceRay(SceneTlas, RAY_FLAG_NONE, 0xFF, 0, 0, 0, shadowRayDesc, shadowPayload);

    if (!shadowPayload.Missed)
    {
        a_payload.Colour /= 2;
    }
}