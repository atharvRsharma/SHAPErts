#version 330 core
out vec4 FragColor;
uniform vec4 spriteColor;

in vec3 Normal;
in vec2 TexCoord;

void main() {
    // --- Simple lighting for 3D look ---
    vec3 lightDir = normalize(vec3(-0.5, -0.8, -0.3));
    float diff = max(dot(normalize(Normal), -lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    vec3 ambient = vec3(0.3);
    FragColor = vec4((ambient + diffuse), 1.0) * spriteColor;
}