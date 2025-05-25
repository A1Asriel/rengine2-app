#version 330 core
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform bool useTexture;
uniform bool distort;
uniform float u_time;
uniform vec3 u_light_color;
uniform vec3 u_light_position;
uniform vec3 u_camera_position;
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;


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

    for(int i=0; i<4; i++) {
        total += noise(p * frequency) * amplitude;
        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return total;
}

void main() {
    if (!gl_FrontFacing) {
        discard;
        return;
    }
    vec4 texColor;
    if (useTexture) {
        if (distort) {
            vec3 noise_coord = vec3(TexCoord * 5.0, u_time * 0.1);
            float noise_value = perlin(noise_coord) * 0.1;
            vec2 distorted_uv = TexCoord + vec2(noise_value);
            texColor = texture(textureSampler, distorted_uv);
        } else {
            texColor = texture(textureSampler, TexCoord);
        }
    } else {
        texColor = vec4(1.0, 1.0, 1.0, 1.0);
    }

    vec3 ambient = u_light_color * material.ambient;

    vec3 norm = normalize(Normal);
    vec3 light_direction = normalize(u_light_position - FragPos);
    float diff = max(dot(norm, light_direction), 0.0);
    vec3 diffuse = u_light_color * (diff * material.diffuse);

    vec3 viewDir = normalize(u_camera_position - FragPos);
    vec3 reflectDir = reflect(-light_direction, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = u_light_color * (spec * material.specular);

    FragColor = texColor * vec4(ambient + diffuse + specular, 1.0);
}
