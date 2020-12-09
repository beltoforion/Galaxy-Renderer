#pragma once

#include <vector>
#include <string>

#include <gl/glew.h>
#include <glm/glm.hpp>

struct Color 
{
	float r, g, b, a;
};

struct VertexColor
{
	float x, y, z;
	float red, green, blue, alpha;
};


class VertexBufferBase
{
public:
	VertexBufferBase();
	virtual ~VertexBufferBase();

	void Initialize();
	void Release();
	void CreateBuffer(const std::vector<VertexColor>& vert, const std::vector<int>& idx, GLuint primitiveType) noexcept(false);
	void UpdateBuffer(const std::vector<VertexColor>& vert) noexcept(false);
	void Draw(glm::mat4& matView, glm::mat4& matProj);

	virtual void OnBeforeDraw();
	virtual void OnSetupAttribArray() const = 0;
	virtual void OnReleaseAttribArray() const = 0;

protected:
	GLuint _bufferMode;

	virtual const char* GetVertexShaderSource() const = 0;
	virtual const char* GetFragmentShaderSource() const = 0;

private:

	// Vertex buffer object
	GLuint _vbo;

	// Index Buffer Object
	GLuint _ibo;

	// vertex array object
	GLuint _vao;

	std::vector<VertexColor> _vert;
	std::vector<int> _idx;

	GLuint _vertexShader;
	GLuint _fragmentShader;
	GLuint _shaderProgram;

	GLuint _primitiveType;

	GLuint CreateShader(GLenum shaderType, const char** shaderSource);
};