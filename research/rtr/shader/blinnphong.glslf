var blinnphongglslf=`
precision mediump float;

uniform vec3 uLightPos;             // Light position in camera space
uniform float uLightPower;          // Light power
uniform vec3 uDiffuseColor;         // Diffuse color
uniform vec3 uSpecularColor;        // Specular color
uniform float uExponent;            // Blinn-Phong exponent
uniform float uAmbient;             // Ambient

varying vec3 vPosition;             // Fragment position (camera space)
varying vec3 vNormal;               // Fragment normal (camera space)

void main(void) 
{
    vec3 i = uLightPos - vPosition;
    vec3 o_u = -normalize(vPosition);
        
    vec3 norm_u = normalize(vNormal);
    vec3 i_u = normalize(i);
        
    vec3 half_vec = normalize(i_u+o_u);
        
    float power = uLightPower/(dot(i,i)/5.0+5.0);
    float rad = power*max(dot(i_u,norm_u),0.0);
    float e = pow(max(dot(half_vec,norm_u),0.0),uExponent);

    gl_FragColor = vec4(uDiffuseColor*(rad + uAmbient) + sign(rad)*power*e*uSpecularColor,1.0);
}
`;
