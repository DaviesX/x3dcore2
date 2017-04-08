var lightdirectionglslf=`
precision mediump float;

uniform vec3 uLightPos;             // Light position in camera space

varying vec3 vPosition;             // Fragment position (camera space)
varying vec3 vNormal;               // Fragment normal (camera space)    

void main(void) 
{
    // Dummy variable to ensure the use of all vertex attributes.
    vec4 zero = vec4(vPosition + vNormal - vPosition - vNormal, 0.0);
        
    vec3 i = uLightPos-vPosition;
    gl_FragColor = zero + vec4(normalize(i),1.0);
}
`;
