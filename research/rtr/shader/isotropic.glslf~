var isotropicglslf=`
precision mediump float;

uniform vec3 uLightPos;             // Light position in camera space
uniform float uLightPower;          // Light power
uniform vec3 uDiffuseColor;         // Diffuse color
uniform vec3 uSpecularColor;        // Specular color
uniform float uBeta;                // Roughness
uniform float uIOR;                 // Index of refraction
uniform float uAmbient;             // Ambient

varying vec3 vPosition;             // Fragment position (camera space)
varying vec3 vNormal;               // Fragment normal (camera space)

const float PI = 3.1415926535897932384626433832795;

float x2(float x)
{
    return x*x;
}

float x3(float x)
{
    return x*x*x;
}

float x4(float x)
{
    return x*x*x*x;
}

float fresnel(vec3 i, vec3 h) 
{
    float c = dot(i,h);
    float g = sqrt(x2(uIOR) - 1.0 + x2(c));
    return 0.5*x2((g-c)/(g+c))*(1.0+x2((c*(g+c)-1.0)/(c*(g-c)+1.0)));
}

float ggx_distri(vec3 n, vec3 h) 
{
    float cos_th = dot(n,h);
    float tan_th2 = 1.0/x2(cos_th)-1.0;
    float b2 = x2(uBeta);
    return b2/(PI*x4(cos_th)*x2(b2+tan_th2));
}

float ggx_shadow1(vec3 v, vec3 h)
{
    float tan_tv2 = 1.0/x2(dot(v,h))-1.0;
    float b2 = x2(uBeta);
    return 2.0/(1.0+sqrt(1.0+b2*tan_tv2));
}

float ggx_shadow(vec3 i, vec3 o, vec3 h) 
{
    return ggx_shadow1(i,h)*ggx_shadow1(o,h);
}

void main(void) 
{
    vec3 i = uLightPos - vPosition;
    vec3 o_u = -normalize(vPosition);
    
    vec3 norm_u = normalize(vNormal);
    vec3 i_u = normalize(i);
    
    vec3 half_vec = normalize(i_u+o_u);
    
    float power = uLightPower/(dot(i,i)/5.0+5.0);
    float rad = power*max(dot(i_u,norm_u),0.0);
    
    vec3 r = rad*(uDiffuseColor + uSpecularColor*fresnel(i_u,half_vec)*ggx_distri(norm_u,half_vec)*ggx_shadow(i_u,o_u,norm_u)/(4.0*(dot(norm_u,i_u)+0.00001*(1.0-sign(rad)))*dot(norm_u,o_u)));

    gl_FragColor = vec4(r+uDiffuseColor*uAmbient, 1.0);
}
`;
