var stdmeshglslv = `
uniform mat4 uMVMatrix;             // Model-view matrix
uniform mat4 uPMatrix;              // Projection matrix
uniform mat4 uNMatrix;              // Normal matrix

attribute vec3 aVertexPosition;     // Vertex position in object space
attribute vec3 aVertexNormal;       // Vertex normal in object space

varying vec3 vPosition;             // Vertex position (camera space)
varying vec3 vNormal;               // Vertex normal (camera space)

void main(void) 
{
    vec4 camSpacePosition = uMVMatrix * vec4(aVertexPosition, 1.0);
    vPosition = vec3(camSpacePosition);

    gl_Position = uPMatrix * camSpacePosition;        

    vec4 camSpaceNormal = uNMatrix * vec4(aVertexNormal, 0.0);
    vNormal = vec3(camSpaceNormal);
}
`;
