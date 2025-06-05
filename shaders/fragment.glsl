#version 330 core
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

out vec4 FragColor;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

#define POINT_LIGHTS_MAX 128

uniform bool distort;
uniform float u_time;
uniform vec3 u_camera_position;
uniform DirLight dirLight;
uniform PointLight pointLights[POINT_LIGHTS_MAX];
uniform Material material;
uniform int pointLightsCount;

vec3 random3(vec3 p) {
    return fract(
        sin(vec3(
            dot(p, vec3(127.1, 311.7, 74.7)),
            dot(p, vec3(269.5, 183.3, 246.1)),
            dot(p, vec3(113.5, 271.9, 124.6))
        )) * 43758.5453
    );
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f*f*(3.0-2.0*f);

    return mix(
        mix(mix(dot(random3(i), f), 
                dot(random3(i + vec3(1.0,0.0,0.0)),f - vec3(1.0,0.0,0.0)),f.x),
            mix(dot(random3(i + vec3(0.0,1.0,0.0)),f - vec3(0.0,1.0,0.0)),
                dot(random3(i + vec3(1.0,1.0,0.0)),f - vec3(1.0,1.0,0.0)),f.x),f.y),
        mix(mix(dot(random3(i + vec3(0.0,0.0,1.0)),f - vec3(0.0,0.0,1.0)),
                dot(random3(i + vec3(1.0,0.0,1.0)),f - vec3(1.0,0.0,1.0)),f.x),
            mix(dot(random3(i + vec3(0.0,1.0,1.0)),f - vec3(0.0,1.0,1.0)),
                dot(random3(i + vec3(1.0,1.0,1.0)),f - vec3(1.0,1.0,1.0)),f.x),f.y),f.z);
}

float perlin(vec3 p) {
    float total = 0.0;
    float frequency = 1.0;
    float amplitude = 1.0;

    for(int i=0; i<pointLightsCount; i++) {
        total += noise(p * frequency) * amplitude;
        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return total;
}

vec3 getAmbient(vec3 ambient, vec2 ourUV) {
    return ambient * vec3(texture(material.diffuse, ourUV));
}

vec3 getDiffuse(vec3 diffuse, vec3 norm, vec3 light_direction, vec2 ourUV) {
    float diff = max(dot(norm, light_direction), 0.0);
    return diffuse * diff * vec3(texture(material.diffuse, ourUV));
}

vec3 getSpecular(vec3 specular, vec3 norm, vec3 viewDir, vec3 light_direction, vec2 ourUV) {
    vec3 halfwayDir = normalize(light_direction + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    return specular * spec * vec3(texture(material.specular, ourUV));
}

vec3 getDirLight(vec3 norm, vec3 viewDir, vec2 ourUV) {
    vec3 light_direction = normalize(-dirLight.direction);

    vec3 ambient = getAmbient(dirLight.ambient, ourUV);
    vec3 diffuse = getDiffuse(dirLight.diffuse, norm, light_direction, ourUV);
    vec3 specular = getSpecular(dirLight.specular, norm, viewDir, light_direction, ourUV);

    return ambient + diffuse + specular;
}

vec3 getPointLight(PointLight light, vec3 norm, vec3 viewDir, vec2 ourUV) {
    vec3 light_direction = normalize(light.position - FragPos);

    vec3 ambient = getAmbient(light.ambient, ourUV);
    vec3 diffuse = getDiffuse(light.diffuse, norm, light_direction, ourUV);
    vec3 specular = getSpecular(light.specular, norm, viewDir, light_direction, ourUV);

    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    return (ambient + diffuse + specular) * attenuation;
}

void main() {
    if (!gl_FrontFacing) {
        discard;
        return;
    }

    vec2 ourUV;

    if (distort) {
        vec3 noise_coord = vec3(TexCoord * 5.0, u_time * 0.1);
        float noise_value = perlin(noise_coord) * 0.1;
        ourUV = TexCoord + vec2(noise_value);
    } else {
        ourUV = TexCoord;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(u_camera_position - FragPos);
    vec3 dirLight = getDirLight(norm, viewDir, ourUV);
    vec3 pointLight = vec3(0.0);
    for (int i = 0; i < pointLightsCount; i++) {
        pointLight += getPointLight(pointLights[i], norm, viewDir, ourUV);
    }
    FragColor = vec4(dirLight + pointLight, 1.0);
}
