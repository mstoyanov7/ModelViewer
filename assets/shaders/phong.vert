#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec3 aCol;
layout(location=3) in vec2 aUV;

out vec3 vCol;
out vec3 vNormal;
out vec3 vWorldPos;
out vec2 vUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat3 uNormalMat;

void main(){
    vec4 worldPos = uModel * vec4(aPos,1.0);
    vWorldPos = worldPos.xyz;
    vNormal   = normalize(uNormalMat * aNormal);
    vCol      = aCol;
    vUV       = aUV;
    gl_Position = uProj * uView * worldPos;
}
