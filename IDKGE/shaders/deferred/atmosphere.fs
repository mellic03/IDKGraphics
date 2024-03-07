#version 460 core

#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;

#include "../UBOs/UBOs.glsl"
#include "../include/SSBO_indirect.glsl"

#include "../include/util.glsl"
#include "../include/pbr.glsl"
#include "../include/lightsource.glsl"

#include "../include/atmospheric.glsl"


in vec3 fsin_fragpos;
flat in int idk_AtmosphereID;


// uniform sampler2D un_texture_0;
// uniform sampler2D un_texture_1;
// uniform sampler2D un_texture_2;

uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;


#define ATMOSPHERE_SAMPLES_A    16
#define ATMOSPHERE_SAMPLES_B    8
#define ATMOSPHERE_INTENSITY    1.0



struct IDK_Sphere
{
    vec3  position;
    float radius;
};


vec3 IDK_RaySphereIntersection_SurfaceNormal( vec3 origin, vec3 dir, IDK_Sphere sphere )
{
    vec3  L   = sphere.position - origin;
    float l   = length(L);
    float r   = sphere.radius;
    float tc  = dot(dir, L);
    float d   = sqrt(l*l - tc*tc);
    float t1c = sqrt(r*r - d*d);

    if (d >= r)
    {
        return -dir;
    }

    float t1 = tc - t1c;
    float t2 = tc + t1c;
    vec3  P  = origin + t1*dir;

    return normalize(P - sphere.position);
}


bool IDK_RaySphereIntersection_IsInside( vec3 origin, IDK_Sphere sphere )
{
    return distance(origin, sphere.position) <= sphere.radius;
}




float IDK_RaySphereIntersection_Distance( vec3 origin, vec3 dir, IDK_Sphere sphere )
{
    vec3  L   = sphere.position - origin;
    float l   = length(L);
    float r   = sphere.radius;
    float tc  = dot(dir, L);
    float d   = sqrt(l*l - tc*tc);
    float t1c = sqrt(r*r - d*d);

    if (d >= r)
    {
        return -1.0;
    }

    float t1 = tc - t1c;
    float t2 = tc + t1c;

    return (t1 < 0.0) ? t2 : t1;
}


vec2 IDK_RaySphereIntersection_MinMaxDistance( vec3 origin, vec3 dir, IDK_Sphere sphere )
{
    vec3  L   = sphere.position - origin;
    float l   = length(L);
    float r   = sphere.radius;
    float tc  = dot(dir, L);
    float d   = sqrt(l*l - tc*tc);
    float t1c = sqrt(r*r - d*d);

    if (d >= r)
    {
        return vec2(-1.0);
    }

    float t1 = max(tc - t1c, 0.0);
    float t2 = max(tc + t1c, 0.0);

    return vec2(t1, t2);
}



float IDK_RaySphereIntersection_MinDistance( vec3 origin, vec3 dir, IDK_Sphere sphere )
{
    vec3  L   = sphere.position - origin;
    float l   = length(L);
    float r   = sphere.radius;
    float tc  = dot(dir, L);
    float d   = sqrt(l*l - tc*tc);
    float t1c = sqrt(r*r - d*d);

    if (d >= r)
    {
        return -1.0;
    }

    float t1 = tc - t1c;
    float t2 = tc + t1c;

    return max(t1, t2);
}


float IDK_RaySphereIntersection_MaxDistance( vec3 origin, vec3 dir, IDK_Sphere sphere )
{
    vec3  L   = sphere.position - origin;
    float l   = length(L);
    float r   = sphere.radius;
    float tc  = dot(dir, L);
    float d   = sqrt(l*l - tc*tc);
    float t1c = sqrt(r*r - d*d);

    if (d >= r)
    {
        return -1.0;
    }

    float t1 = tc - t1c;
    float t2 = tc + t1c;

    return max(t1, t2);
}


float IDK_RaySphereIntersection_DistanceThroughSphere( vec3 origin, vec3 dir, IDK_Sphere sphere )
{
    float A = IDK_RaySphereIntersection_Distance(origin, dir, sphere);
    float B = IDK_RaySphereIntersection_MaxDistance(origin, dir, sphere);

    if (A < 0.0 || B < 0.0)
    {
        return -1.0;
    }

    return B - A;

}


IDK_Sphere  OuterAtmosphere  = { vec3(0.0, 0.0, 0.0), 0.0 };
IDK_Sphere  SurfaceSphere    = { vec3(0.0, 0.0, 0.0), 0.0 };
float       MaxAltitude      = 0.0;
float       DENSITY_FALLOFF  = 1.0;


float IDK_SphereAltitude( vec3 position, IDK_Sphere sphere )
{
    return length(position - sphere.position) - sphere.radius;
}


float ComputeDensity( vec3 position )
{
    float altitude = IDK_SphereAltitude(position, SurfaceSphere);
    float d = 1.0 - clamp(altitude / MaxAltitude, 0.0, 1.0);

    return d * exp(-altitude * DENSITY_FALLOFF);
}


float ComputeOpticalDepth( vec3 origin, vec3 L, float distToAtmosphere, IDK_Camera camera )
{
    float step_size = distToAtmosphere / ATMOSPHERE_SAMPLES_B;
    float od = 0.0;

    for (int i=0; i<ATMOSPHERE_SAMPLES_B; i++)
    {
        vec3 ray_pos = origin + float(i) * L;
        od += ComputeDensity(ray_pos);
    }

    return od * step_size;
}


vec3 ComputeTransmittance( float sunOpticalDepth, float viewOpticalDepth, vec3 coefficients )
{
    return exp(-(sunOpticalDepth + viewOpticalDepth) * coefficients);
}





int yeahboye( vec3 dir )
{
    const vec3 directions[6] = vec3[]
    (
        vec3( +1.0,  0.0,  0.0 ),
        vec3( -1.0,  0.0,  0.0 ),
        vec3(  0.0, +1.0,  0.0 ),
        vec3(  0.0, -1.0,  0.0 ),
        vec3(  0.0,  0.0, +1.0 ),
        vec3(  0.0,  0.0, -1.0 )
    );


    int   best_idx = -1;
    float best_dot = -1.0;

    for (int i=0; i<6; i++)
    {
        float dot = dot(dir, directions[i]);

        if (dot > best_dot)
        {
            best_dot = dot;
            best_idx = i;
        }
    }

    return best_idx;
}


vec2 yeahboye2( int face, vec3 dir )
{
    dir = abs(dir);
    vec2 texcoord;

    // X-axis
    if (face < 2)
    {
        texcoord = vec2(1.0 - dir.z, 1.0 - dir.y);
    }

    // Y-axis
    else if (face < 4)
    {
        texcoord = vec2(1.0 - dir.x, 1.0 - dir.z);
    }

    // Z-axis
    else
    {
        texcoord = vec2(1.0 - dir.x, 1.0 - dir.y);
    }

    return mod((texcoord - 0.5) * 2.0, 1.0);
}


ivec3 cube_ree( vec3 v, float face )
{
	vec3 vAbs = abs(v);
	float ma;
	vec2 uv;

	if (vAbs.z >= vAbs.x && vAbs.z >= vAbs.y)
	{
		face = v.z < 0.0 ? 5.0 : 4.0;
		ma = 0.5 / vAbs.z;
		uv = vec2(v.z < 0.0 ? -v.x : v.x, -v.y);
	}

	else if (vAbs.y >= vAbs.x)
	{
		face = v.y < 0.0 ? 3.0 : 2.0;
		ma = 0.5 / vAbs.y;
		uv = vec2(v.x, v.y < 0.0 ? -v.z : v.z);
	}

	else
	{
		face = v.x < 0.0 ? 1.0 : 0.0;
		ma = 0.5 / vAbs.x;
		uv = vec2(v.x < 0.0 ? v.z : -v.z, -v.y);
	}

	return ivec3(256.0 * (uv * ma + 0.5), face);
}




vec3 ComputeOcean( vec3 viewpos, vec3 ray_start, vec3 ray_dir, vec3 sampled_pos, IDK_Atmosphere atmosphere )
{
    vec3  color = vec3(0.05, 0.2, 0.8);

    return color;
}



void main()
{
    vec3 scattering = vec3(0.0);
    vec3 diffuse    = un_dirlights[0].diffuse.xyz;

    IDK_Camera     camera    = IDK_RenderData_GetCamera();
    IDK_Atmosphere atmosphere = un_RenderData.atmospheres[idk_AtmosphereID];

    OuterAtmosphere.position = atmosphere.position.xyz;
    SurfaceSphere.position   = atmosphere.position.xyz;
    OuterAtmosphere.radius   = atmosphere.radius * atmosphere.scale;
    SurfaceSphere.radius     = atmosphere.radius;

    MaxAltitude     = OuterAtmosphere.radius - SurfaceSphere.radius;
    DENSITY_FALLOFF = atmosphere.density_falloff;

    const vec3 ScatterCoefficients = vec3(
        atmosphere.scatter_strength * pow(400.0 / atmosphere.wavelengths[0], 4.0),
        atmosphere.scatter_strength * pow(400.0 / atmosphere.wavelengths[1], 4.0),
        atmosphere.scatter_strength * pow(400.0 / atmosphere.wavelengths[2], 4.0)
    );


    vec3  viewpos   = camera.position.xyz;
    vec3  frag_pos  = fsin_fragpos;
    float frag_dist = distance(viewpos, frag_pos);
    vec3  ray_dir   = normalize(frag_pos - viewpos);

    vec2 texcoord = IDK_WorldToUV(frag_pos, camera.PV).xy;
    vec3 sampled_pos = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);

    vec3  L        = -normalize(atmosphere.position.xyz - atmosphere.sun_position.xyz);
    float cosTheta = max(dot(ray_dir, L), 0.0);

    vec2  atmosphere_dists = IDK_RaySphereIntersection_MinMaxDistance(viewpos, ray_dir, OuterAtmosphere);
    float surface_dist     = distance(viewpos, sampled_pos);
    float ray_dist         = min(atmosphere_dists[1], surface_dist - atmosphere_dists[0]);

    if (ray_dist <= 0.0)
    {
        fsout_frag_color = vec4(0.0);
        return;
    }


    const vec3  start_pos = viewpos + atmosphere_dists[0]*ray_dir;
    const vec3  end_pos   = start_pos + ray_dist*ray_dir;
    const float step_size = ray_dist / ATMOSPHERE_SAMPLES_A;
    const vec3  mie       = IDK_AtmosphericMie(cosTheta);
    const vec3  rayleigh  = IDK_AtmosphericRayleigh(cosTheta);


    vec3 ray_pos  = start_pos;
    vec3 ray_step = step_size * ray_dir;

    for (int i=0; i<ATMOSPHERE_SAMPLES_A; i++)
    {
        float distToAtmosphere = IDK_RaySphereIntersection_Distance(ray_pos, L, OuterAtmosphere);
        float sunOpticalDepth  = ComputeOpticalDepth(ray_pos, L, distToAtmosphere, camera);
        float viewOpticalDepth = ComputeOpticalDepth(ray_pos, -ray_dir, float(i) * step_size, camera);
        vec3  transmittance    = ComputeTransmittance(sunOpticalDepth, viewOpticalDepth, ScatterCoefficients);
        float density          = ComputeDensity(ray_pos);

        scattering += density * transmittance;

        ray_pos += ray_step;
    }


    vec3 atmospherics = scattering * ScatterCoefficients * step_size;

    if (atmosphere_dists[1] < distance(viewpos, sampled_pos))
    {
        atmospherics *= mie + rayleigh;
    }


    float alpha = (distance(viewpos, end_pos)) / atmosphere.radius;
          alpha = clamp(alpha, 0.0, 1.0);

    vec3 result = alpha*atmospherics;

    fsout_frag_color = vec4(result, 1.0);
}



