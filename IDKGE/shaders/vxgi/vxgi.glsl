#define FORMAT_RGBA16F 0
#define FORMAT_RGBA8UI 1

#define VXGI_TEXTURE_FORMAT         FORMAT_RGBA16F
#define VXGI_TEXTURE_SIZE           256.0
#define VXGI_WORLD_BOUNDS           32.0
#define VXGI_WORLD_HALF_BOUNDS      (VXGI_WORLD_BOUNDS / 2.0)
#define VXGI_VOXEL_SIZE             (VXGI_WORLD_BOUNDS / VXGI_TEXTURE_SIZE)
#define VXGI_VOXEL_SCALE            (VXGI_TEXTURE_SIZE / VXGI_WORLD_BOUNDS)
#define VXGI_MAX_MIPLEVEL           4.0




vec3 VXGI_truncatePosition( vec3 position )
{
    return floor(VXGI_VOXEL_SCALE * position) / VXGI_VOXEL_SCALE;
}


vec3 VXGI_truncateViewPosition( vec3 view_position )
{
    // float scale = SCALE / int(pow(2, 4));
    // return floor(scale * view_position) / scale;
    // return VXGI_truncatePosition(view_position);
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

    vec3 p = VXGI_truncatePosition(world_position);
         p += VXGI_WORLD_HALF_BOUNDS;
         p /= VXGI_VOXEL_SIZE;

    return ivec3(p);

}



vec3 VXGI_TexelToWorld( ivec3 texel, vec3 view_position )
{
    vec3 v = VXGI_truncateViewPosition(view_position);

    vec3 p = vec3(texel);

    p *= VXGI_VOXEL_SIZE;
    p -= VXGI_WORLD_HALF_BOUNDS;
    p += v;

    return p;
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


ivec3 VXGI_encodeNormal( vec3 N )
{
    return ivec3(255.0 * (N * 0.5 + 0.5));
}


vec3 VXGI_decodeNormal( ivec3 N )
{
    return (vec3(N) / 255.0) * 2.0 - 1.0;
}


ivec3 VXGI_AnisoIndex3( vec3 dir )
{
    return ivec3(0);

    ivec3 idx;

    idx[0] = (dir.x < 0.0) ? 0 : 1;
    idx[1] = (dir.y < 0.0) ? 2 : 3;
    idx[2] = (dir.z < 0.0) ? 4 : 5;

    return idx;
}


vec3 VXGI_AnisoWeights( vec3 N )
{
    // return vec3(
    //     dot(N, vec3(sign(N.x), 0.0, 0.0)) * 0.5 + 0.5,
    //     dot(N, vec3(0.0, sign(N.y), 0.0)) * 0.5 + 0.5,
    //     dot(N, vec3(0.0, 0.0, sign(N.z))) * 0.5 + 0.5
    // );

    return N * N;
}


vec4 VXGI_AnisoTextureLod( sampler3D aniso[6], vec3 N, vec3 texcoord, float level )
{
    ivec3 idx = VXGI_AnisoIndex3(N);
    vec3  wgt = VXGI_AnisoWeights(N);

    vec4 result = vec4(0.0);

    const vec3 directions[6] = vec3[]
    (
        vec3(-1.0, 0.0, 0.0),
        vec3(+1.0, 0.0, 0.0),
        vec3(0.0, -1.0, 0.0),
        vec3(0.0, +1.0, 0.0),
        vec3(0.0, 0.0, -1.0),
        vec3(0.0, 0.0, +1.0)
    );

    for (int i=0; i<6; i++)
    {
        float weight = dot(N, directions[i]);
        weight = (weight > -0.01) ? 1.0/3.0 : 0.0;

        vec4 ree = textureLod(aniso[i], texcoord, level);
        result += vec4(weight * ree.rgb, ree.a);
    }

    // for (int i=0; i<3; i++)
    // {
    //     vec4 ree = textureLod(aniso[idx[i]], texcoord, level);
    //     result += vec4(wgt[i]*ree.rgb, ree.a);
    // }

    return result;
}


vec3 VXGI_TraceCone( vec3 origin, vec3 dir, float aperture, vec3 view_position, sampler3D voxeldata )
{
    vec4  color = vec4(0.0);
    float dist  = 8.0 * VXGI_VOXEL_SIZE;

    float diameter;
    float mipLevel = 0.0;

    while (color.a < 1.0 && mipLevel < VXGI_MAX_MIPLEVEL)
    {
        diameter = 2.0 * dist * tan(aperture / 2.0);
        mipLevel = log2(diameter / VXGI_VOXEL_SIZE);

        vec3 position = origin + dist*dir;

        if (VXGI_in_bounds(position, view_position) == false)
        {
            break;
        }

        vec3 texcoord = VXGI_WorldToTexCoord(position, view_position);
        vec4 voxel    = textureLod(voxeldata, texcoord, mipLevel);
            //  voxel.rgb *= pow(1.8, mipLevel);
    
        color.rgb += (1.0 - color.a) * voxel.rgb;
        color.a   += (1.0 - color.a) * voxel.a;
        dist      += diameter;
    }

    return color.rgb;
}


vec3 VXGI_TraceConeAniso( vec3 origin, vec3 dir, float aperture, vec3 view_position,
                          sampler3D normal, sampler3D aniso[6] )
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
            //  voxel.rgb *= pow(2, miplevel);

        color.rgb += (1.0 - color.a) * voxel.a * voxel.rgb;
        color.a   += (1.0 - color.a) * voxel.a;
        dist      += diameter;
    }

    // float attenuation = 1.0 / (1.0 + dist);
    // return color.rgb * attenuation;
    return color.rgb;
}


vec3 VXGI_TraceSpecular( vec3 origin, vec3 dir, float aperture, vec3 view_position, sampler3D voxel_albedo )
{
	vec3 direction = normalize(dir);

	const float OFFSET = 2.0 * VXGI_VOXEL_SIZE;
	const float STEP = VXGI_VOXEL_SIZE;

	vec3 from = origin;

	vec4 acc = vec4(0.0);
	float dist = OFFSET;


	while(dist < VXGI_WORLD_BOUNDS && acc.a < 1){ 
		vec3 c = from + dist * direction;

        if (VXGI_in_bounds(c, view_position) == false)
        {
            break;
        }

		c = VXGI_WorldToTexCoord(c, view_position); 
		
		float level = 0.1 * aperture * log2(1 + dist / VXGI_VOXEL_SIZE);
		vec4 voxel = textureLod(voxel_albedo, c, min(level, VXGI_MAX_MIPLEVEL));
		float f = 1 - acc.a;
		acc.rgb += 0.25 * (1 + aperture) * voxel.rgb * voxel.a * f;
		acc.a += 0.25 * voxel.a * f;
		dist += STEP * (1.0 + 0.125 * level);
	}
	return 1.0 * pow(aperture + 1, 0.8) * acc.rgb;
}



float VXGI_TraceAO( vec3 origin, vec3 dir, float aperture, vec3 view_position, sampler3D voxeldata )
{
    float alpha = 0.0;
    float dist  = 2.0 * VXGI_VOXEL_SIZE;

    for (int i=0; i<3; i++)
    {
        float diameter = 2.0 * dist * tan(aperture / 2.0);
        float mipLevel = log2(diameter / VXGI_VOXEL_SIZE);

        if (mipLevel > VXGI_MAX_MIPLEVEL)
        {
            break;
        }

        vec3 position = origin + dist*dir;

        if (VXGI_in_bounds(position, view_position) == false)
        {
            break;
        }

        vec3 texcoord = VXGI_WorldToTexCoord(position, view_position);
        vec4 voxel    = textureLod(voxeldata, texcoord, mipLevel); // * pow(2, mipLevel);
    
        alpha += (1.0 - alpha) * voxel.a;
        dist  += 1.0 * diameter;
    }

    return alpha;
}


float VXGI_TraceShadow( vec3 origin, vec3 dir, float aperture, vec3 view_position, sampler3D voxeldata )
{
    float alpha = 0.0;
    float dist   = 2.0 * VXGI_VOXEL_SIZE;

    while (alpha < 1.0)
    {
        float diameter = 2.0 * dist * tan(aperture / 2.0);
        float mipLevel = log2(diameter / VXGI_VOXEL_SIZE);

        if (mipLevel > VXGI_MAX_MIPLEVEL)
        {
            break;
        }

        vec3 position = origin + dist*dir;

        if (VXGI_in_bounds(position, view_position) == false)
        {
            break;
        }

        vec3  texcoord = VXGI_WorldToTexCoord(position, view_position);
        vec4  voxel    = textureLod(voxeldata, texcoord, mipLevel);
    
        alpha += (1.0 - alpha) * voxel.a;

        dist += diameter;
    }

    return alpha;
}

