//**********************************
// OpenGL Texture 2D
// 27/08/2009 - 11/03/2011
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include "test.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-320/texture-2d.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/texture-2d.frag");
	char const * TEXTURE_DIFFUSE("rgba512.dds");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer
	
	namespace shader
	{
		enum type
		{
			VERT,
			FRAG,
			MAX
		};
	}//namespace shader

	std::vector<GLuint> ShaderName(shader::MAX);
	GLuint VertexArrayName(0);
	GLuint ProgramName(0);
	GLuint TextureName(0);
	std::vector<GLuint> BufferName(buffer::MAX);
	GLint UniformTransform(0);
	GLint UniformDiffuse(0);
}//namespace

class gl_320_texture2d : public test
{
public:
	gl_320_texture2d(int argc, char* argv[]) :
		test(argc, argv, "gl-320-texture-2d", test::CORE, 3, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated(true);
	
		if(Validated)
		{
			glf::compiler Compiler;
			ShaderName[shader::VERT] = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE, "--version 150 --profile core");
			ShaderName[shader::FRAG] = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE, "--version 150 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, ShaderName[shader::VERT]);
			glAttachShader(ProgramName, ShaderName[shader::FRAG]);

			glBindAttribLocation(ProgramName, glf::semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName, glf::semantic::attr::TEXCOORD, "Texcoord");
			glBindFragDataLocation(ProgramName, glf::semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName);
			Validated = Validated && glf::checkProgram(ProgramName);
		}

		if(Validated)
		{
			UniformTransform = glGetUniformBlockIndex(ProgramName, "transform");
			UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
		}
	
		Validated = Validated && glf::checkError("initProgram");
	
		return Validated;
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);

		glGetIntegerv(
			GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
			&UniformBufferOffset);

		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return glf::checkError("initBuffer");
	}

	bool initTexture()
	{
		gli::texture2D Texture(gli::load_dds((glf::DATA_DIRECTORY + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &TextureName);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture.levels() == 1 ? GL_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(
				GL_TEXTURE_2D,
				GLint(Level),
				gli::internal_format(Texture.format()),
				GLsizei(Texture[Level].dimensions().x),
				GLsizei(Texture[Level].dimensions().y),
				0,
				gli::external_format(Texture.format()),
				gli::type_format(Texture.format()),
				Texture[Level].data());
		}

		if(Texture.levels() == 1)
			glGenerateMipmap(GL_TEXTURE_2D);
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		return glf::checkError("initTexture");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(glf::semantic::attr::POSITION);
			glEnableVertexAttribArray(glf::semantic::attr::TEXCOORD);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return glf::checkError("initVertexArray");
	}

	virtual int begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initVertexArray();

		return Validated ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	virtual int end()
	{
		glDeleteShader(ShaderName[shader::VERT]);
		glDeleteShader(ShaderName[shader::FRAG]);
		glDeleteProgram(ProgramName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return glf::checkError("end") ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	virtual void render()
	{
		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
			glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -this->TranlationCurrent.y));
			glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, this->RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
			glm::mat4 View = glm::rotate(ViewRotateX, this->RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * View * Model;

			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glm::ivec2 WindowSize = this->getWindowSize();

		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glUseProgram(ProgramName);
		glUniform1i(UniformDiffuse, 0);
		glUniformBlockBinding(ProgramName, UniformTransform, glf::semantic::uniform::TRANSFORM0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindVertexArray(VertexArrayName);

		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0);
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_texture2d Test(argc, argv);
	Error += Test();

	return Error;
}