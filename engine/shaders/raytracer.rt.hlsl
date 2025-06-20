#include "[shader]/global.hlsli"

ConstantBuffer<SceneConstantBuffer> SceneCB : register(b0, space0);
RaytracingAccelerationStructure SceneTlas : register(t0, space0);
RWTexture2D<float4> OutRenderTarget : register(u0, space0);

struct Payload
{
    float3 Colour;
    bool AllowReflection; // #Todo: optimize this - can be turned into bitwise flags.
    bool Missed;
};

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 pixelIdx = DispatchRaysIndex().xy;
    const uint2 pixelTotal = DispatchRaysDimensions().xy;

    float2 imagePlanePos = (pixelIdx + 0.5f) / pixelTotal * 2.0f - 1.0f;
    imagePlanePos.y = -imagePlanePos.y;
    
    float4 localRayDirection = mul(float4(imagePlanePos, SceneCB.CameraPosition.w, 0), SceneCB.ProjectionToWorld);
    localRayDirection.xyz /= localRayDirection.w;
    
    const float3 rayOrigin = SceneCB.CameraPosition.xyz;
    const float3 rayDir = localRayDirection.xyz - rayOrigin;
    
    const RayDesc rayDesc =
    {
        rayOrigin,
		0.001,
		rayDir,
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
    a_payload.Colour = SceneCB.SkyColour.xyz;
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

    //HitFloor(a_payload, a_attrib.barycentrics);
}

void HitMesh(inout Payload a_payload, float2 a_uv)
{
    a_payload.Colour = float3(1, 0, 1);
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
		SceneCB.SunDirection.xyz - rayHitPosition,
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