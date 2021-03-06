#version 330 core
out vec4 FragColor;

uniform vec4 ourColor;
//in vec2 TexCoord;

// texture samplers
//uniform sampler2D texture1;
//uniform sampler2D texture2;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = ourColor;
}