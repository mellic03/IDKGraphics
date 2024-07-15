#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec3 fsin_fragpos;


uniform samplerCube un_cubemap;
uniform samplerCube un_depthmap;

#define PI 3.14159265359


float linearDepth( float depthSample, float zNear, float zFar )
{
    depthSample = 2.0 * depthSample - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
}



void main()
{
    vec3 N = normalize(fsin_fragpos);
    vec3 irradiance = vec3(0.0);

    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));

    float sampleDelta = 0.06;
    float nrSamples = 0.0;

    for (float phi = 0.0;  phi < 2.0*PI;  phi += sampleDelta)
    {
        for (float theta = 0.0;  theta < 0.5*PI;  theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += (texture(un_cubemap, sampleVec).rgb) * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    // irradiance = texture(un_cubemap, N).rgb;

    fsout_frag_color = vec4(irradiance, 1.0);
}
