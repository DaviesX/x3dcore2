var diffuseglslf=`
precision mediump float;

uniform vec3 uLightPos;             // Light position in camera space
uniform float uLightPower;          // Light power
uniform vec3 uDiffuseColor;         // Diffuse color   
uniform float uAmbient;             // Ambient

varying vec3 vPosition;             // Fragment position (camera space)
varying vec3 vNormal;               // Fragment normal (camera space)

void main(void) 
{
    vec3 i = uLightPos - vPosition;
    float power = uLightPower/(dot(i,i)/5.0+5.0);
    float rad = power*max(dot(normalize(i),normalize(vNormal)),0.0);
    gl_FragColor = vec4(uDiffuseColor*(rad + uAmbient),1.0);
}
`;
