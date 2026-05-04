#include "[shader]/global.hlsli"

/* Space 0 - global resources. */
ConstantBuffer<SceneConstantBuffer> SceneCB : register(b0, space0);
RaytracingAccelerationStructure SceneTlas : register(t0, space0);
RWTexture2D<float4> OutRenderTarget : register(u0, space0);

/* Space 1 - bindless resource heap. */
StructuredBuffer<MeshData> Meshes : register(t0, space1);
StructuredBuffer<Vertex> VertexBuffers[256] : register(t1, space1);
StructuredBuffer<uint> IndexBuffers[256] : register(t257, space1);

struct Triangle
{
    Vertex v0, v1, v2;
};

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 pixel = DispatchRaysIndex().xy;
    const uint2 resolution = DispatchRaysDimensions().xy;
    const float2 imagePlanePos = ((float2)pixel + 0.5f) / resolution * 2.0f - 1.0f;

    const float aspectRatio = SceneCB.Projection[1][1] / SceneCB.Projection[0][0];
    const float tanHalfFovY = 1.0f / SceneCB.Projection[1][1];
    
    RayDesc rayDesc;
    rayDesc.Origin = SceneCB.View[3].xyz;
    rayDesc.TMin = MinTraceDistance;
    rayDesc.TMax = MaxTraceDistance;
    rayDesc.Direction = normalize(
        (imagePlanePos.x * SceneCB.View[0].xyz * tanHalfFovY * aspectRatio) -
        (imagePlanePos.y * SceneCB.View[1].xyz * tanHalfFovY) +
        SceneCB.View[2].xyz
    );

    HitInfo primaryHitInfo;
    primaryHitInfo.Colour = float3(0.0f, 0.0f, 0.0f);
    primaryHitInfo.AllowReflection = true;
    primaryHitInfo.Distance = 0.0f;
    primaryHitInfo.Missed = false;

    TraceRay(SceneTlas, RAY_FLAG_NONE, 0xFF, 0, 0, 0, rayDesc, primaryHitInfo);

    OutRenderTarget[pixel] = float4(primaryHitInfo.Colour, 1);
}

[shader("closesthit")]
void ClosestHit(inout HitInfo a_payload, BuiltInTriangleIntersectionAttributes a_attrib)
{
    a_payload.Distance += RayTCurrent();
    
    MeshData mesh = Meshes[InstanceID()];
    const StructuredBuffer<Vertex> vertices = VertexBuffers[NonUniformResourceIndex(mesh.VertexBufferIdx)];
    const StructuredBuffer<u32> indicies = IndexBuffers[NonUniformResourceIndex(mesh.IndexBufferIdx)];
    
    const uint elementIdx = PrimitiveIndex() * 3;
    const Triangle tri = 
    { 
        vertices[indicies[elementIdx + 0]],
        vertices[indicies[elementIdx + 1]],
        vertices[indicies[elementIdx + 2]]
    };
    
    const float3 triangleWeights = float3(1.0f - a_attrib.barycentrics.x - a_attrib.barycentrics.y, a_attrib.barycentrics.x, a_attrib.barycentrics.y);
    
    /* Currently visualizing normals. */
    float3 normal = normalize(tri.v0.Normal * triangleWeights.x + tri.v1.Normal * triangleWeights.y + tri.v2.Normal * triangleWeights.z);
    //float2 uv = v0.UV * baryCoord.x + v1.UV * baryCoord.y + v2.UV * baryCoord.z;
    a_payload.Colour = normal * 0.5f + 0.5f;
}

[shader("miss")]
void Miss(inout HitInfo a_payload)
{
    a_payload.Colour = SceneCB.SkyColour.rgb;
    a_payload.Missed = true;
}