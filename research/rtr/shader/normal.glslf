var normalglslf=`
precision mediump float;

varying vec3 vPosition;             // Vertex position (camera space)
varying vec3 vNormal;               // Fragment normal (camera space)

void main(void) 
{
    // Dummy variable to ensure the use of all vertex attributes.
    vec4 zero = vec4(vPosition + vNormal - vPosition - vNormal, 0.0);
    gl_FragColor = vec4(normalize(vNormal),1.0);
}
`;
