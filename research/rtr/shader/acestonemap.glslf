var acestonemap=`

// in
varying vec2        tex_coords;

uniform sampler2D   hdr_buffer;
uniform float       log2_lum;


vec3 acestonemap(vec3 color, float adapted_lum)
{
    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;
    color *= adapted_lum;
    return (color*(A*color+B))/(color*(C*color+D)+E);
}

void main(void)
{
    vec3 hdr_color = texture(hdr_buffer, tex_coords).rgb;
    gl_FragColor = acestonemap(hdr_color, exp2(log2_lum)); 
}
`;
