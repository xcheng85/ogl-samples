//**********************************
// OpenGL Render to texture
// 20/10/2009 - 01/11/2011
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	char const * VERTEX_SHADER_SOURCE("gl-400/fbo-rtt.vert");
	char const * FRAGMENT_SHADER_SOURCE("gl-400/fbo-rtt.frag");
	int const FRAMEBUFFER_SIZE(2);
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);

	glf::window Window("gl-400-fbo-rtt", glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(3);

	enum texture_type
	{
		TEXTURE_R,
		TEXTURE_G,
		TEXTURE_B,
		TEXTURE_MAX
	};

	GLuint FramebufferName(0);
	GLuint VertexArrayName(0);
	GLuint ProgramName(0);
	GLint UniformDiffuse(0);

	GLuint TextureName[TEXTURE_MAX] = {0, 0, 0};
	glm::vec4 Viewport[TEXTURE_MAX];
}//namespace

bool initProgram()
{
	bool Validated = true;

	if(Validated)
	{
		GLuint VertexShaderName = glf::createShader(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERTEX_SHADER_SOURCE);
		GLuint FragmentShaderName = glf::createShader(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAGMENT_SHADER_SOURCE);

		Validated = Validated && glf::checkShader(VertexShaderName, VERTEX_SHADER_SOURCE);
		Validated = Validated && glf::checkShader(FragmentShaderName, FRAGMENT_SHADER_SOURCE);

		ProgramName = glCreateProgram();
		glAttachShader(ProgramName, VertexShaderName);
		glAttachShader(ProgramName, FragmentShaderName);
		glDeleteShader(VertexShaderName);
		glDeleteShader(FragmentShaderName);
		glLinkProgram(ProgramName);
		Validated = Validated && glf::checkProgram(ProgramName);
	}

	if(Validated)
	{
		UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
	}

	return Validated && glf::checkError("initProgram");
}

bool initTexture2D()
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(TEXTURE_MAX, TextureName);

	for(int i = TEXTURE_R; i <= TEXTURE_B; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, TextureName[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			GL_RGBA8,
			GLsizei(Window.Size.x / FRAMEBUFFER_SIZE),
			GLsizei(Window.Size.y / FRAMEBUFFER_SIZE),
			0,  
			GL_RGB, 
			GL_UNSIGNED_BYTE, 
			0);
	}

	return glf::checkError("initTexture");
}

bool initFramebuffer()
{
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, TextureName[1], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, TextureName[2], 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return glf::checkError("initFramebuffer");
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
	glBindVertexArray(VertexArrayName);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	glm::ivec2 const FramebufferSize(Window.Size / FRAMEBUFFER_SIZE);
	
	Viewport[TEXTURE_R] = glm::vec4(Window.Size.x >> 1, 0, FramebufferSize);
	Viewport[TEXTURE_G] = glm::vec4(Window.Size.x >> 1, Window.Size.y >> 1, FramebufferSize);
	Viewport[TEXTURE_B] = glm::vec4(0, Window.Size.y >> 1, FramebufferSize);

	bool Validated = true;

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initVertexArray();
	if(Validated)
		Validated = initTexture2D();
	if(Validated)
		Validated = initFramebuffer();
	
	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteTextures(TEXTURE_MAX, TextureName);
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteProgram(ProgramName);

	return glf::checkError("end");
}

void display()
{
	GLint const Border(16);
	glm::ivec2 const FramebufferSize(Window.Size / FRAMEBUFFER_SIZE);

	glViewport(0, 0, FramebufferSize.x, FramebufferSize.y);

	// Pass 1
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	GLenum DrawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, DrawBuffers);

	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)[0]);
	glClearBufferfv(GL_COLOR, 1, &glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)[0]);
	glClearBufferfv(GL_COLOR, 2, &glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)[0]);

	// Pass 2
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	glUseProgram(ProgramName);
	glUniform1i(UniformDiffuse, 0);

	glBindVertexArray(VertexArrayName);

	for(std::size_t i = 0; i < TEXTURE_MAX; ++i)
	{
		glViewport(
			GLint(Viewport[i].x), GLint(Viewport[i].y), 
			GLsizei(Viewport[i].z), GLsizei(Viewport[i].w));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[i]);

		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
	}

	glUseProgram(0);
}

int main(int argc, char* argv[])
{
	return glf::run(argc, argv, glf::CORE, 4, 0);
}
