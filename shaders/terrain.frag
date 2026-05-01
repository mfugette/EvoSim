#version 330 core

in vec3 FragPos;
in vec3 Normal;
in float Height;

out vec4 FragColor;

uniform sampler2D uDeepWater;
uniform sampler2D uShallow;
uniform sampler2D uSand;
uniform sampler2D uGrass;
uniform sampler2D uForest;
uniform sampler2D uRock;
uniform sampler2D uSnow;

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

uniform float texScale;

vec3 blendColors(vec3 a, vec3 b, float t)
{
    return mix(a, b, clamp(t, 0.0, 1.0));
}

float heightBlend(float lo, float hi)
{
    return smoothstep(lo, hi, Height);
}

vec3 terrainColor(vec2 uv)
{
    vec3 colDeepWater = texture(uDeepWater, uv).rgb;
    vec3 colShallow = texture(uShallow, uv).rgb;
    vec3 colSand = texture(uSand, uv).rgb;
    vec3 colGrass = texture(uGrass, uv).rgb;
    vec3 colForest = texture(uForest, uv).rgb;
    vec3 colRock = texture(uRock, uv).rgb;
    vec3 colSnow = texture(uSnow, uv).rgb;

    vec3 col = colDeepWater;
    col = blendColors(col, colShallow, heightBlend(heightDeepWater, heightShallow));
    col = blendColors(col, colSand, heightBlend(heightShallow, heightSand));
    col = blendColors(col, colGrass, heightBlend(heightSand, heightGrass));
    col = blendColors(col, colForest, heightBlend(heightGrass, heightForest));
    col = blendColors(col, colRock, heightBlend(heightForest, heightRock));
    col = blendColors(col, colSnow, heightBlend(heightRock, heightSnow));

    float slope = 1.0 - abs(Normal.y);
    float rockBlend = smoothstep(0.55, 0.75, slope);
    float aboveSand = smoothstep(heightSand, heightGrass, Height);

    col = blendColors(col, colRock, rockBlend * aboveSand);

    return col;
}

void main()
{
    vec2 uv = FragPos.xz * texScale;

    vec3 norm = normalize(Normal);
    vec3 baseColor = terrainColor(uv);

    float ambientStrength = 0.18;
    vec3 ambient = ambientStrength * lightColor * baseColor;

    float diff = max(dot(norm, normalize(lightDir)), 0.0);
    vec3 diffuse = diff * lightColor * baseColor;

    float specStrength = 0.04;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfDir = normalize(normalize(lightDir) + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), 16.0);
    vec3 specular = specStrength * spec * lightColor;

    vec3 lighting = ambient + diffuse + specular;

    float dist = length(viewPos - FragPos);
    float fogFactor = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
    vec3 finalColor = mix(fogColor, lighting, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}