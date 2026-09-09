// EXTRACTED printable GLSL fragments from Main.exe.
// File offsets: 0x0078F3B1..0x0078F4E1
// No invented statements; NUL-separated literals were joined with newlines.
out vec2 textureCoordinate;
void main()
    const vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0));
    vec2 position = positions[gl_VertexID];
    textureCoordinate = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0.9999, 1.0);
