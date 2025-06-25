#include "[shader]/global.hlsli"

// Global resources - extend outside of the bindless resource bindings.
ConstantBuffer<SceneConstantBuffer> SceneCB : register(b0, space0);
RaytracingAccelerationStructure SceneTlas : register(t0, space0);
RWTexture2D<float4> OutRenderTarget : register(u0, space0);

// Bindless resources
ByteAddressBuffer Vertices[] : register(t0, space1);
ByteAddressBuffer Indices[] : register(t0, space2);
//Texture2D<float4> Textures[] : register(t0, space3); - #Todo: implement texture loading.
//SamplerState LinearSampler : register(s0, space0);


// --- Ray generation ---

RayDesc GeneratePrimaryRay(in const float2 a_imagePlanePos)
{
    RayDesc rayDesc;
    rayDesc.Origin = SceneCB.View[3].xyz;
    rayDesc.TMin = MinTraceDistance;
    rayDesc.TMax = MaxTraceDistance;

    const float aspectRatio = SceneCB.Projection[1][1] / SceneCB.Projection[0][0];
    const float tanHalfFovY = 1.0f / SceneCB.Projection[1][1];
    
    rayDesc.Direction = normalize(
        (a_imagePlanePos.x * SceneCB.View[0].xyz * tanHalfFovY * aspectRatio) -
        (a_imagePlanePos.y * SceneCB.View[1].xyz * tanHalfFovY) +
        SceneCB.View[2].xyz
    );
    
    return rayDesc;
}


// --- Hit responses ---

void HitMesh(inout HitInfo a_payload, float2 a_uv)
{
    const float f = (InstanceIndex() % 255);
    a_payload.Colour = float3(f / 255.f, f / 255.f, f / 255.f);
}

void HitMirror(inout HitInfo a_payload, float2 a_uv)
{
    if (!a_payload.AllowReflection)
    {
        return;
    }

    const float3 rayHitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    const float3 worldNormal = normalize(mul(float3(0, 1, 0), (float3x3) ObjectToWorld4x3()));
    const float3 reflectedWorldDirection = reflect(normalize(WorldRayDirection()), worldNormal);

    RayDesc mirrorRayDesc;
    mirrorRayDesc.Origin = rayHitPosition;
    mirrorRayDesc.TMin = MinTraceDistance;
    mirrorRayDesc.Direction = reflectedWorldDirection;
    mirrorRayDesc.TMax = MaxTraceDistance;

    a_payload.AllowReflection = false;
    TraceRay(SceneTlas, RAY_FLAG_NONE, 0xFF, 0, 0, 0, mirrorRayDesc, a_payload);
}

void HitFloor(inout HitInfo a_payload, float2 a_uv)
{
    const float3 rayHitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    const bool2 checkerPattern = frac(rayHitPosition.xz) > 0.5;
    a_payload.Colour = checkerPattern.x ^ checkerPattern.y ? 0.6.xxx : 0.4.xxx;
	
    // #Todo: Create a second hitgroup that's to be used for shadows with an anyhit shader - this is also something that shouldn't happen only to the floor.   
    RayDesc shadowRayDesc;
    shadowRayDesc.Origin = rayHitPosition;
    shadowRayDesc.TMin = MinTraceDistance;
    shadowRayDesc.Direction = SceneCB.SunDirection.xyz - rayHitPosition;
    shadowRayDesc.TMax = MaxTraceDistance;
    
    HitInfo shadowHitInfo;
    shadowHitInfo.Colour = float3(0.0f, 0.0f, 0.0f),
    shadowHitInfo.AllowReflection = false;
    shadowHitInfo.Distance = 0.0f;
    shadowHitInfo.Missed = false;
    
    TraceRay(SceneTlas, RAY_FLAG_NONE, 0xFF, 0, 0, 0, shadowRayDesc, shadowHitInfo);

    if (!shadowHitInfo.Missed)
    {
        a_payload.Colour /= 2;
    }
}


// --- Raytrace shaders ---

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 pixel = DispatchRaysIndex().xy;
    const uint2 resolution = DispatchRaysDimensions().xy;

    const float2 imagePlanePos = ((pixel + 0.5f) / resolution) * 2.0f - 1.0f;
    RayDesc primaryRayDesc = GeneratePrimaryRay(imagePlanePos);
    
    HitInfo primaryHitInfo;
    primaryHitInfo.Colour = float3(0.0f, 0.0f, 0.0f),
    primaryHitInfo.AllowReflection = true;
    primaryHitInfo.Distance = 0.0f;
    primaryHitInfo.Missed = false;

    TraceRay(SceneTlas, RAY_FLAG_NONE, 0xFF, 0, 0, 0, primaryRayDesc, primaryHitInfo);
    OutRenderTarget[pixel] = float4(primaryHitInfo.Colour, 1);
}

[shader("closesthit")]
void ClosestHit(inout HitInfo a_payload, BuiltInTriangleIntersectionAttributes a_attrib)
{
    a_payload.Distance += RayTCurrent();
    
    switch (InstanceID())
    {
    case 0:
        HitFloor(a_payload, a_attrib.barycentrics);
        break;
    case 1:
        HitMirror(a_payload, a_attrib.barycentrics);
        break;
    default:
        HitMesh(a_payload, a_attrib.barycentrics);
        break;
    }
}

[shader("miss")]
void Miss(inout HitInfo a_payload)
{
    a_payload.Colour = SceneCB.SkyColour.xyz;
    a_payload.Missed = true;
}