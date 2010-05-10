#version 330 core

// Declare all the semantics
#define ATTR_POSITION	0
#define ATTR_COLOR		3
#define ATTR_TEXCOORD	4
#define VERT_POSITION	0
#define VERT_COLOR		3
#define VERT_TEXCOORD	4
#define VERT_INSTANCE	7
#define FRAG_COLOR		0
#define FRAG_RED		0
#define FRAG_GREEN		1
#define FRAG_BLUE		2
#define FRAG_ALPHA		3

uniform sampler2D Diffuse;

in vert
{
	/*layout(location = VERT_TEXCOORD)*/ vec2 Texcoord;
} Vert;


layout(location = FRAG_COLOR) out vec4 Color;

void main()
{
	Color = texture(Diffuse, Vert.Texcoord);
}
