// GLSL 1.20 (SFML 2.6.x)
uniform sampler2D texture;
uniform float u_mix; // 0..1  (1 => fully white)

void main() {
    vec4 tex = texture2D(texture, gl_TexCoord[0].xy) * gl_Color;
    vec3 col = mix(tex.rgb, vec3(1.0, 1.0, 1.0), clamp(u_mix, 0.0, 1.0));
    gl_FragColor = vec4(col, tex.a);
}
