#version 460 core

in vec2 p_uv;

uniform sampler2D screen_image;

out vec4 frag_color;

void main()
{
	frag_color = texture(screen_image, p_uv);
}