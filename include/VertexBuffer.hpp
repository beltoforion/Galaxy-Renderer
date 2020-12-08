#pragma once

#include <vector>
#include <string>

#include <gl/glew.h>
#include <glm/glm.hpp>

struct Color 
{
	Color(float a_r = 1, float a_g = 1, float a_b = 1, float a_a = 1)
	{
		r = a_r;
		g = a_g;
		b = a_b;
		a = a_a;
	}

	float r, g, b, a;
};

struct VertexColor
{
	float x, y, z;
	float red, green, blue, alpha;
};

class VertexBuffer
{
public:
	VertexBuffer(int lineWidth = 1);
	~VertexBuffer();

	void Initialize();
	void Release();
	void Update(const std::vector<VertexColor>& vert, const std::vector<int>& idx, GLuint primitiveType) noexcept(false);
	void Draw(glm::mat4& matView, glm::mat4& matProj);

private:

	enum AttributeIdx : int
	{
		attPosition = 0,
		attColor = 1
	};

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

	int _lineWidth;

	GLuint CreateShader(GLenum shaderType, const char** shaderSource);
};