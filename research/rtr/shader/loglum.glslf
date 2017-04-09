var loglumglslf=`
varying vec2        tex_coords;

uniform sampler2D   hdr_buffer;



float color2lum(vec3 c)
{
    const vec3 w(0.299,0.587,0.114);
    return dot(w,c);
}

void main(void)
{
    vec3 hdr_color = texture(hdr_buffer, tex_coords).rgb;
    gl_FragColor = log2(color2lum(hdr_color) + 0.001);
}
`;
