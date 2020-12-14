#include "VertexBufferBase.hpp"
#include <stdexcept>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

VertexBufferBase::VertexBufferBase()
	: _vbo(0)
	, _ibo(0)
	, _vao(0)
	, _bufferMode(GL_STATIC_DRAW)
	, _vert()
	, _idx()
	, _shaderProgram(0)
	, _primitiveType(0)
{}

VertexBufferBase::~VertexBufferBase()
{}

GLuint VertexBufferBase::CreateShader(GLenum shaderType, const char** shaderSource)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, shaderSource, nullptr);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

		std::string msg(infoLog.data());

		// We don't need the shader anymore.
		glDeleteShader(shader);

		throw std::runtime_error(std::string("VertecBuffer: Shader compilation failed: ") + msg);
	}

	return shader;
}

void VertexBufferBase::Initialize()
{
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ibo);
	glGenVertexArrays(1, &_vao);

	const char *srcVertex = GetVertexShaderSource();
	GLuint vertexShader = CreateShader(GL_VERTEX_SHADER, &srcVertex);

	const char *srcFragment = GetFragmentShaderSource();
	GLuint fragmentShader = CreateShader(GL_FRAGMENT_SHADER, &srcFragment);

	_shaderProgram = glCreateProgram();
	glAttachShader(_shaderProgram, vertexShader);
	glAttachShader(_shaderProgram, fragmentShader);
	glLinkProgram(_shaderProgram);

	GLint isLinked = 0;
	glGetProgramiv(_shaderProgram, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(_shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(_shaderProgram, maxLength, &maxLength, &infoLog[0]);

		// clean up
		glDeleteProgram(_shaderProgram);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		throw std::runtime_error("VertexBuffer: shader program linking failed!");
	}

	// Always detach shaders after a successful link.
	glDetachShader(_shaderProgram, vertexShader);
	glDetachShader(_shaderProgram, fragmentShader);
}

void VertexBufferBase::Release()
{
	OnReleaseAttribArray();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (_vbo != 0)
		glDeleteBuffers(1, &_vbo);

	if (_ibo != 0)
		glDeleteBuffers(1, &_ibo);

	if (_vao != 0)
		glDeleteVertexArrays(1, &_vao);
}

void VertexBufferBase::UpdateBuffer(const std::vector<VertexColor>& vert) noexcept(false)
{
	if (_bufferMode == GL_STATIC_DRAW)
		throw std::runtime_error("VertexBufferBase: static buffers cannot be updated!");

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _vert.size() * sizeof(VertexColor), vert.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBufferBase::CreateBuffer(const std::vector<VertexColor>& vert, const std::vector<int>& idx, GLuint type) {
	_vert = vert;
	_idx = idx;
	_primitiveType = type;

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _vert.size() * sizeof(VertexColor), _vert.data(), _bufferMode);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

	// Set up vertex buffer array
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	OnSetupAttribArray();

	// Set up index buffer array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _idx.size() * sizeof(int), _idx.data(), GL_STATIC_DRAW);

	auto errc = glGetError();
	if (errc != GL_NO_ERROR)
	{
		std::stringstream ss;
		ss << "VertexBuffer: Cannot create vbo! (Error 0x" << std::hex << errc << ")" << std::endl;
		throw std::runtime_error(ss.str());
	}

	glBindVertexArray(0);
}

void VertexBufferBase::OnBeforeDraw()
{
}

void VertexBufferBase::Draw(glm::mat4& matView, glm::mat4& matProjection)
{
	glUseProgram(_shaderProgram);

	GLuint viewMatIdx = glGetUniformLocation(_shaderProgram, "viewMat");
	glUniformMatrix4fv(viewMatIdx, 1, GL_FALSE, glm::value_ptr(matView));

	GLuint projMatIdx = glGetUniformLocation(_shaderProgram, "projMat");
	glUniformMatrix4fv(projMatIdx, 1, GL_FALSE, glm::value_ptr(matProjection));

	glEnable(GL_PRIMITIVE_RESTART);
	glEnable(GL_BLEND);
	glPrimitiveRestartIndex(0xFFFF);

	OnBeforeDraw();

	glBindVertexArray(_vao);
	glDrawElements(_primitiveType, (int)_idx.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glDisable(GL_PRIMITIVE_RESTART);
	glUseProgram(0);
}

