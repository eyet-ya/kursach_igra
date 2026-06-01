#version 120

uniform sampler2D texture0;

uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

varying vec3 Normal;
varying vec3 FragPos;
varying vec2 TexCoord;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Ambient
    vec3 ambient = lightAmbient * materialAmbient;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightDiffuse * (diff * materialDiffuse);
    
    // Specular
    vec3 viewDir = normalize(-FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = lightSpecular * (spec * materialSpecular);
    
    vec4 texColor = texture2D(texture0, TexCoord);
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    
    gl_FragColor = vec4(result, texColor.a);
}