#version 330 core
in vec3 vCol;
in vec3 vNormal;
in vec3 vWorldPos;

out vec4 FragColor;

uniform vec3 uLightDir;    // normalized, world space (direction towards surface)
uniform vec3 uViewPos;     // camera position, world space
uniform bool uUseLighting;

void main(){
    vec3 baseColor = vCol;

    if(!uUseLighting){
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    // Phong lighting
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(R, V), 0.0), 32.0);

    vec3 ambient  = 0.12 * baseColor;
    vec3 diffuse  = 0.88 * diff * baseColor;
    vec3 specular = 0.25 * spec * vec3(1.0);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}

