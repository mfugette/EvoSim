#version 330 core

in vec3 FragPos;
in vec3 Normal;
in float Height;

out vec4 FragColor;

uniform vec3 lightDir;       
uniform vec3 lightColor;     
uniform vec3 viewPos;       

uniform float heightDeepWater;
uniform float heightShallow;
uniform float heightSand;
uniform float heightGrass;
uniform float heightForest;
uniform float heightRock;
uniform float heightSnow;

uniform float fogStart;
uniform float fogEnd;
uniform vec3  fogColor;

vec3 blendColors(vec3 a, vec3 b, float t)
{
    return mix(a, b, clamp(t, 0.0, 1.0));
}

float heightBlend(float lo, float hi)
{
    return smoothstep(lo, hi, Height);
}

vec3 terrainColor()
{
    vec3 colDeepWater = vec3(0.05, 0.15, 0.35);
    vec3 colShallow   = vec3(0.10, 0.30, 0.55);
    vec3 colSand      = vec3(0.76, 0.70, 0.50);
    vec3 colGrass     = vec3(0.25, 0.52, 0.18);
    vec3 colForest    = vec3(0.10, 0.32, 0.10);
    vec3 colRock      = vec3(0.45, 0.40, 0.35);
    vec3 colSnow      = vec3(0.92, 0.95, 1.00);

    vec3 col = colDeepWater;
    col = blendColors(col, colShallow, heightBlend(heightDeepWater, heightShallow));
    col = blendColors(col, colSand,    heightBlend(heightShallow,   heightSand));
    col = blendColors(col, colGrass,   heightBlend(heightSand,      heightGrass));
    col = blendColors(col, colForest,  heightBlend(heightGrass,     heightForest));
    col = blendColors(col, colRock,    heightBlend(heightForest,    heightRock));
    col = blendColors(col, colSnow,    heightBlend(heightRock,      heightSnow));

    float slope = 1.0 - abs(Normal.y);
    float rockBlend = smoothstep(0.55, 0.75, slope);

    float aboveSand = smoothstep(heightSand, heightGrass, Height);
    col = blendColors(col, colRock, rockBlend * aboveSand);

    return col;
}

void main()
{
    vec3 norm     = normalize(Normal);
    vec3 baseColor = terrainColor();

    float ambientStrength = 0.18;
    vec3  ambient = ambientStrength * lightColor * baseColor;

    float diff    = max(dot(norm, normalize(lightDir)), 0.0);
    vec3  diffuse = diff * lightColor * baseColor;

    float specStrength = 0.04;
    vec3  viewDir  = normalize(viewPos - FragPos);
    vec3  halfDir  = normalize(normalize(lightDir) + viewDir);
    float spec     = pow(max(dot(norm, halfDir), 0.0), 16.0);
    vec3  specular = specStrength * spec * lightColor;

    vec3 lighting = ambient + diffuse + specular;

    float dist      = length(viewPos - FragPos);
    float fogFactor = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
    vec3  finalColor = mix(fogColor, lighting, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}