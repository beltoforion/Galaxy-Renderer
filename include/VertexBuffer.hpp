#pragma once

#include <vector>
#include <string>

#include <gl/glew.h>


struct VertexColor
{
	float x, y, z;
	float red, green, blue;
};

class VertexBuffer
{
public:
	VertexBuffer(int lineWidth = 1);
	~VertexBuffer();

	void Initialize();
	void Release();
	void Update(const std::vector<VertexColor>& vert, const std::vector<int>& idx) noexcept(false);
	void Draw();

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
	int _lineWidth;

	GLuint CreateShader(GLenum shaderType, const char** shaderSource);
};