#version 120

varying vec3 Normal;
varying vec3 FragPos;
varying vec2 TexCoord;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    FragPos = (gl_ModelViewMatrix * gl_Vertex).xyz;
    Normal = gl_NormalMatrix * gl_Normal;
    TexCoord = gl_MultiTexCoord0.xy;
}