#define FORMAT_RGBA16F 0
#define FORMAT_RGBA8UI 1

#define VXGI_TEXTURE_FORMAT         FORMAT_RGBA16F
#define VXGI_TEXTURE_SIZE           64.0
#define VXGI_WORLD_BOUNDS           16.0
#define VXGI_WORLD_HALF_BOUNDS      (VXGI_WORLD_BOUNDS / 2.0)
#define VXGI_VOXEL_SIZE             (VXGI_WORLD_BOUNDS / VXGI_TEXTURE_SIZE)
#define VXGI_VOXEL_SCALE            (VXGI_TEXTURE_SIZE / VXGI_WORLD_BOUNDS)
#define VXGI_MAX_MIPLEVEL           6.0


#define VXGI_DIFFUSE_APERTURE 60.0
#define VXGI_DIFFUSE_OFFSET   1.25
#define VXGI_DIFFUSE_STRENGTH 0.25

#define VXGI_SPECULAR_APERTURE 120.0
#define VXGI_SPECULAR_OFFSET   1.25
#define VXGI_SPECULAR_STRENGTH 0.25


const vec3[6] VXGI_ANISO_DIRECTIONS = vec3[6]
(
    vec3(-1.0,  0.0,  0.0),
    vec3(+1.0,  0.0,  0.0),
    vec3( 0.0, -1.0,  0.0),
    vec3( 0.0, +1.0,  0.0),
    vec3( 0.0,  0.0, -1.0),
    vec3( 0.0,  0.0, +1.0)
);



vec3 VXGI_truncatePosition( vec3 position )
{
    return floor(VXGI_VOXEL_SCALE * (position + VXGI_VOXEL_SIZE / 2)) / VXGI_VOXEL_SCALE;
}


vec3 VXGI_truncateViewPosition( vec3 view_position )
{
    float scale = VXGI_VOXEL_SCALE; // / int(pow(2, 4));
    // return floor(scale * view_position) / scale;
    // return view_position;
    return vec3(0.0);
}


bool VXGI_in_bounds( vec3 pos, vec3 view_position )
{
    vec3 v = VXGI_truncateViewPosition(view_position);
    pos -= v;

    return pos.x > (-VXGI_WORLD_HALF_BOUNDS) && pos.x < (VXGI_WORLD_HALF_BOUNDS)
        && pos.y > (-VXGI_WORLD_HALF_BOUNDS) && pos.y < (VXGI_WORLD_HALF_BOUNDS)
        && pos.z > (-VXGI_WORLD_HALF_BOUNDS) && pos.z < (VXGI_WORLD_HALF_BOUNDS);
}


ivec3 VXGI_WorldToTexel( vec3 world_position, vec3 view_position )
{
    vec3 v = VXGI_truncateViewPosition(view_position);

    vec3 position = VXGI_truncatePosition(world_position) - v;
         position += VXGI_WORLD_BOUNDS / 2.0;
         position /= VXGI_VOXEL_SIZE;

    ivec3 texel = ivec3(position);

    return texel;
}



vec3 VXGI_TexelToWorld( ivec3 texel, vec3 view_position )
{
    vec3 v = VXGI_truncateViewPosition(view_position);

    vec3 position = vec3(texel);
         position *= VXGI_VOXEL_SIZE;
         position -= VXGI_WORLD_BOUNDS / 2.0;
         position += v;
    
    return position;
}


vec3 VXGI_WorldToTexCoord( vec3 world_position, vec3 view_position )
{
    ivec3 texel = VXGI_WorldToTexel(world_position, view_position);
    return vec3(texel) / VXGI_TEXTURE_SIZE;
}


vec3 VXGI_TexCoordToWorld( vec3 texcoord, vec3 view_position )
{
    ivec3 texel = ivec3(texcoord * VXGI_TEXTURE_SIZE);
    return VXGI_TexelToWorld(texel, view_position);
}


ivec3 VXGI_PackNormal( vec3 N )
{
    return ivec3(255.0 * (N * 0.5 + 0.5));
}


vec3 VXGI_UnpackNormal( ivec3 N )
{
    return (vec3(N) / 255.0) * 2.0 - 1.0;
}


uvec3 VXGI_AnisoIndex3( vec3 dir )
{
    uvec3 idx = uvec3(0);

    idx[0] = (dir.x < 0.0) ? 0 : 1;
    idx[1] = (dir.y < 0.0) ? 2 : 3;
    idx[2] = (dir.z < 0.0) ? 4 : 5;

    return idx;
}


vec3 VXGI_AnisoWeights( vec3 dir )
{
    return normalize(dir * dir);
}




vec4 VXGI_AnisoTextureLod( sampler3D aniso[6], vec3 dir, vec3 texcoord, float level )
{
    const vec3 directions[6] = vec3[]
    (
        vec3(-1.0,  0.0,  0.0),
        vec3(+1.0,  0.0,  0.0),
        vec3( 0.0, -1.0,  0.0),
        vec3( 0.0, +1.0,  0.0),
        vec3( 0.0,  0.0, -1.0),
        vec3( 0.0,  0.0, +1.0)
    );

    vec4 result = vec4(0.0);


    uvec3 indices = VXGI_AnisoIndex3(dir);
    vec3  weights = VXGI_AnisoWeights(dir);

    for (int i=0; i<6; i++)
    {
        uint  index  = i; // indices[i];
        float weight = max(dot(dir, directions[index]), 0.0);

        vec4  color  = textureLod(aniso[index], texcoord, level);
        result += vec4(weight * color.rgb, color.a);
    }


    return result;
}


vec3 VXGI_TraceCone( vec3 origin, vec3 dir, float aperture, vec3 view_position, sampler3D aniso[6] )
{
    vec4  color = vec4(0.0);
    float dist  = 2.0 * VXGI_VOXEL_SIZE;

    float diameter;
    float miplevel = 0.0;

    while (color.a < 1.0)
    {
        diameter = 2.0 * dist * tan(aperture / 2.0);
        miplevel = log2(diameter / VXGI_VOXEL_SIZE);

        if (miplevel >= VXGI_MAX_MIPLEVEL)
        {
            break;
        }

        vec3 position = origin + dist*dir;

        if (VXGI_in_bounds(position, view_position) == false)
        {
            break;
        }

        vec3 texcoord = VXGI_WorldToTexCoord(position, view_position);
        vec4 voxel    = VXGI_AnisoTextureLod(aniso, -dir, texcoord, miplevel);

        // color.rgb += (1.0 - color.a) * voxel.a * voxel.rgb;
        // color.a += (1.0 - color.a) * voxel.a;
        color += (1.0 - color.a) * voxel;

        dist += diameter;
    }

    return color.rgb;
}


vec3 VXGI_Orthogonal( vec3 u )
{
	u = normalize(u);
	vec3 v = vec3(0.99146, 0.11664, 0.05832);
	return abs(dot(u, v)) > 0.99999 ? cross(u, vec3(0, 1, 0)) : cross(u, v);
}


vec3 VXGI_IndirectDiffuse( vec3 origin, vec3 N, float noise, vec3 viewpos, sampler3D aniso[6] )
{
    vec3  result = vec3(0.0);

    // vec4 noiseA = textureLod(un_voxel_noise, (origin), 0.0);
    // vec4 noiseB = textureLod(un_voxel_noise, origin + noiseA.rgb * un_increment * N, 0.0);

    vec4  alphaA = vec4(0.5); // + vec4(0.25) * (noiseB);
    vec4  alphaB = vec4(0.5); // + vec4(0.25) * (noiseB);

    vec3 T  = normalize(VXGI_Orthogonal(N));
    vec3 B  = normalize(cross(N, T));
    vec3 Tp = normalize(mix(T,  B, 0.5));
    vec3 Bp = normalize(mix(T, -B, 0.5));

    vec3 cone_directions[] = vec3[]
    (
        N,
    
        normalize(mix(N,  T,  alphaA[0])),
        normalize(mix(N,  B,  alphaA[1])),
        normalize(mix(N, -T,  alphaA[2])),
        normalize(mix(N, -B,  alphaA[3])),
    
        normalize(mix(N,  Tp, alphaB[0])),
        normalize(mix(N, -Tp, alphaB[1])),
        normalize(mix(N,  Bp, alphaB[2])),
        normalize(mix(N, -Bp, alphaB[3]))
    );


    for (int i=0; i<9; i++)
    {
        float aperture = radians(60.0);
        vec3  cone_dir = cone_directions[i];
        float weight   = dot(N, cone_dir) * 0.5 + 0.5;

        result += weight * VXGI_TraceCone(
            origin + VXGI_VOXEL_SIZE * N,
            cone_dir,
            aperture,
            viewpos,
            aniso
        );
    }

    return result;
}



vec3 VXGI_TraceCone2( vec3 origin, vec3 dir, float aperture, vec3 view_position, sampler3D aniso[6], samplerCube skybox )
{
    vec4  color = vec4(0.0);
    float dist  = 2.0 * VXGI_VOXEL_SIZE;

    float diameter;
    float miplevel = 0.0;

    while (color.a < 1.0)
    {
        diameter = 2.0 * dist * tan(aperture / 2.0);
        miplevel = log2(diameter / VXGI_VOXEL_SIZE);

        if (miplevel >= VXGI_MAX_MIPLEVEL)
        {
            break;
        }

        vec3 position = origin + dist*dir;

        if (VXGI_in_bounds(position, view_position) == false)
        {
            color = textureLod(skybox, dir, max(miplevel, 5.0));
            break;
        }

        vec3 texcoord = VXGI_WorldToTexCoord(position, view_position);
        vec4 voxel    = VXGI_AnisoTextureLod(aniso, -dir, texcoord, miplevel);

        color += (1.0 - color.a) * voxel;
        dist  += 0.5 * diameter;
    }

    return color.rgb;
}



float VXGI_TraceShadow( vec3 origin, vec3 dir, float aperture, vec3 view_position, sampler3D aniso[6] )
{
    float alpha = 0.0;
    float dist  = 2.0 * VXGI_VOXEL_SIZE;

    float diameter;
    float miplevel = 0.0;

    while (alpha < 1.0)
    {
        diameter = 2.0 * dist * tan(aperture / 2.0);
        miplevel = log2(diameter / VXGI_VOXEL_SIZE);

        if (miplevel >= VXGI_MAX_MIPLEVEL)
        {
            break;
        }

        vec3 position = origin + dist*dir;

        if (VXGI_in_bounds(position, view_position) == false)
        {
            break;
        }

        vec3 texcoord = VXGI_WorldToTexCoord(position, view_position);
        vec4 voxel    = VXGI_AnisoTextureLod(aniso, -dir, texcoord, miplevel);

        alpha += (1.0 - alpha) * voxel.a;
        dist  += 0.5 * diameter;
    }

    return 1.0 - alpha;
}

