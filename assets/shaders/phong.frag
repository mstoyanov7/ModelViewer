#version 330 core
in vec3 vCol;
in vec3 vNormal;
in vec3 vWorldPos;
in vec2 vUV;

out vec4 FragColor;

uniform vec3 uLightDir;    // normalized, world space (direction towards surface)
uniform vec3 uViewPos;     // camera position, world space
uniform bool uUseLighting;
uniform bool uHasBaseColorTex;
uniform sampler2D uBaseColorTex;
uniform vec4 uBaseColorFactor; // glTF baseColorFactor (rgb multiplicative, a used for blending)

void main(){
    vec4 base = vec4(vCol, 1.0);
    if (uHasBaseColorTex) {
        base = texture(uBaseColorTex, vUV);
    }
    base *= uBaseColorFactor;

    if(!uUseLighting){
        FragColor = base;
        return;
    }

    // Phong lighting
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(R, V), 0.0), 32.0);

    vec3 baseColor = base.rgb;
    vec3 ambient  = 0.12 * baseColor;
    vec3 diffuse  = 0.88 * diff * baseColor;
    vec3 specular = 0.25 * spec * vec3(1.0);

    FragColor = vec4(ambient + diffuse + specular, base.a);
}
