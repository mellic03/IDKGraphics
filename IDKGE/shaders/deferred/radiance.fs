#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"
#include "../GI/GI.glsl"


layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoord;

uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;
uniform sampler2D un_fragdepth;


float linearDepth( float z, float near, float far )
{
    return near * far / (far + z * (near - far));
}


vec3 IDK_CubemapParallax( vec3 CameraWS, vec3 PositionWS, vec3 NormalWS, vec3 probe_pos )
{
    vec3 DirectionWS = PositionWS - CameraWS;
    vec3 ReflDirectionWS = reflect(DirectionWS, NormalWS);

    // Following is the parallax-correction code
    // Find the ray intersection with box plane

    vec3 BoxMax = probe_pos + vec3(1.0); // 6.5, 3.3, 2.35) / 2.0;
    vec3 BoxMin = probe_pos - vec3(1.0); // 6.5, 3.3, 2.35) / 2.0;

    vec3 FirstPlaneIntersect = (BoxMax - PositionWS) / ReflDirectionWS;
    vec3 SecondPlaneIntersect = (BoxMin - PositionWS) / ReflDirectionWS;
    // Get the furthest of these intersections along the ray
    // (Ok because x/0 give +inf and -x/0 give –inf )
    vec3 FurthestPlane = max(FirstPlaneIntersect, SecondPlaneIntersect);
    // Find the closest far intersection
    float Distance = min(min(FurthestPlane.x, FurthestPlane.y), FurthestPlane.z);

    // Get the intersection position
    vec3 IntersectPositionWS = PositionWS + ReflDirectionWS * Distance;
    // Get corrected reflection
    ReflDirectionWS = IntersectPositionWS - probe_pos;
    // End parallax-correction code

    return ReflDirectionWS;
}



vec3 IDK_GI_SampleProbeNearest( vec3 world, vec3 N, vec3 R )
{
    world /= IDK_PROBE_CELL_SPACING;
    world += IDK_PROBE_GRID_HSIZE;
    world += round(N);
    world = clamp(world, vec3(0), vec3(IDK_PROBE_GRID_SIZE));

    ivec3 cell = ivec3(round(world));
          cell = clamp(cell, ivec3(0), IDK_PROBE_GRID_SIZE);

    return IDK_GI_SampleGrid(cell, R);
}


vec3 IDK_GI_SampleProbeBlended( vec3 world, vec3 N, vec3 R )
{
    ivec3 offsets[] = ivec3[](
        ivec3( 0, 0, 0 ),
        ivec3( 0, 0, 1 ),
        ivec3( 0, 1, 0 ),
        ivec3( 0, 1, 1 ),
        ivec3( 1, 0, 0 ),
        ivec3( 1, 0, 1 ),
        ivec3( 1, 1, 0 ),
        ivec3( 1, 1, 1 )
    );

    world /= IDK_PROBE_CELL_SPACING;
    world += IDK_PROBE_GRID_HSIZE;
    world = clamp(world, vec3(0.0), vec3(IDK_PROBE_GRID_SIZE));

    ivec3 cell = ivec3(world);
          cell = clamp(cell, ivec3(0), IDK_PROBE_GRID_SIZE);

    vec3 result = vec3(0.0);

    float factors[6] = float[]( 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 );
    float total      = 0.0;

    for (int i=0; i<6; i++)
    {
        vec3 n = normalize(cell + offsets[i] - world);
        factors[i] = max(dot(n, R), 0.0);
        total += factors[i];
    }

    for (int i=0; i<6; i++)
    {
        result += (factors[i] / total) * IDK_GI_SampleGrid(cell + offsets[i], R);
    }

    return result;
}


vec3 IDK_GI_SampleProbeTrillinear( vec3 world, vec3 N, vec3 R )
{
    ivec3 offsets[] = ivec3[](
        ivec3( 0, 0, 0 ),
        ivec3( 0, 0, 1 ),
        ivec3( 0, 1, 0 ),
        ivec3( 0, 1, 1 ),
        ivec3( 1, 0, 0 ),
        ivec3( 1, 0, 1 ),
        ivec3( 1, 1, 0 ),
        ivec3( 1, 1, 1 )
    );

    world /= IDK_PROBE_CELL_SPACING;
    world += IDK_PROBE_GRID_HSIZE;
    world = clamp(world, vec3(0.0), vec3(IDK_PROBE_GRID_SIZE));

    ivec3 cell = ivec3(world);
          cell = clamp(cell, ivec3(0), IDK_PROBE_GRID_SIZE);

    vec3 c000 = IDK_GI_SampleGrid(cell + offsets[0], N);
    vec3 c001 = IDK_GI_SampleGrid(cell + offsets[1], N);
    vec3 c010 = IDK_GI_SampleGrid(cell + offsets[2], N);
    vec3 c011 = IDK_GI_SampleGrid(cell + offsets[3], N);
    vec3 c100 = IDK_GI_SampleGrid(cell + offsets[4], N);
    vec3 c101 = IDK_GI_SampleGrid(cell + offsets[5], N);
    vec3 c110 = IDK_GI_SampleGrid(cell + offsets[6], N);
    vec3 c111 = IDK_GI_SampleGrid(cell + offsets[7], N);

    vec3 n000 = normalize(cell + offsets[0] - world);
    vec3 n001 = normalize(cell + offsets[1] - world);
    vec3 n010 = normalize(cell + offsets[2] - world);
    vec3 n011 = normalize(cell + offsets[3] - world);
    vec3 n100 = normalize(cell + offsets[4] - world);
    vec3 n101 = normalize(cell + offsets[5] - world);
    vec3 n110 = normalize(cell + offsets[6] - world);
    vec3 n111 = normalize(cell + offsets[7] - world);

    float d000 = dot(n000, N);
    float d001 = dot(n001, N);
    float d010 = dot(n010, N);
    float d011 = dot(n011, N);
    float d100 = dot(n100, N);
    float d101 = dot(n101, N);
    float d110 = dot(n110, N);
    float d111 = dot(n111, N);

    vec3 fracCoord = world - cell;
    vec3 oneMinusFrac = vec3(1.0) - fracCoord;

    float w000 = oneMinusFrac.x * oneMinusFrac.y * oneMinusFrac.z;
    float w001 = oneMinusFrac.x * oneMinusFrac.y * fracCoord.z;
    float w010 = oneMinusFrac.x * fracCoord.y * oneMinusFrac.z;
    float w011 = oneMinusFrac.x * fracCoord.y * fracCoord.z;
    float w100 = fracCoord.x * oneMinusFrac.y * oneMinusFrac.z;
    float w101 = fracCoord.x * oneMinusFrac.y * fracCoord.z;
    float w110 = fracCoord.x * fracCoord.y * oneMinusFrac.z;
    float w111 = fracCoord.x * fracCoord.y * fracCoord.z;

    w000 = (d000 < 0.0) ? 0.0 : w000;
    w001 = (d001 < 0.0) ? 0.0 : w001;
    w010 = (d010 < 0.0) ? 0.0 : w010;
    w011 = (d011 < 0.0) ? 0.0 : w011;
    w100 = (d100 < 0.0) ? 0.0 : w100;
    w101 = (d101 < 0.0) ? 0.0 : w101;
    w110 = (d110 < 0.0) ? 0.0 : w110;
    w111 = (d111 < 0.0) ? 0.0 : w111;

    float totalWeight = w000 + w001 + w010 + w011 + w100 + w101 + w110 + w111;

    if (totalWeight > 0.0)
    {
        w000 /= totalWeight;
        w001 /= totalWeight;
        w010 /= totalWeight;
        w011 /= totalWeight;
        w100 /= totalWeight;
        w101 /= totalWeight;
        w110 /= totalWeight;
        w111 /= totalWeight;
    }

    vec3 result = w000*c000 + w001*c001 + w010*c010 + w011*c011 +
                  w100*c100 + w101*c101 + w110*c110 + w111*c111;

    return result;
}



void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];

    vec2  texcoord = fsin_texcoord;
    vec3  fragpos  = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);

    vec4 albedo = texture(un_texture_0, texcoord).rgba;
    vec3 N      = texture(un_texture_1, texcoord).rgb;
    vec3 ao_r_m = texture(un_texture_2, texcoord).rgb;

    float roughness = ao_r_m[0];
    float metallic  = ao_r_m[1];
    float ao        = ao_r_m[2];

    vec3  V     = normalize(camera.position.xyz - fragpos);
    vec3  R     = reflect(-V, N); 
    vec3  F0    = clamp(mix(vec3(0.04), albedo.rgb, metallic), 0.0, 1.0);
    float NdotV = PBR_DOT(N, V);
    vec3  F     = fresnelSchlickR(NdotV, F0, roughness);
    vec3  Ks    = F;
    vec3  Kd    = (vec3(1.0) - Ks) * (1.0 - metallic);

    vec3 irradiance = IDK_GI_SampleProbeTrillinear(fragpos, N, R);
    vec3 result     = PBR_PI * irradiance * albedo.rgb;

    fsout_frag_color = vec4(result, 1.0);

}



