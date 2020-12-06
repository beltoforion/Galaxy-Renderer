#include "VertexBuffer.hpp"
#include <stdexcept>

VertexBuffer::VertexBuffer(int lineWidth)
	: _vbo(0)
	, _ibo(0)
	, _vao(0)
	, _vert()
	, _idx()
	, _lineWidth(lineWidth)
{}

VertexBuffer::~VertexBuffer()
{}

void VertexBuffer::Initialize()
{
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ibo);
	glGenVertexArrays(1, &_vao);
}

void VertexBuffer::Release()
{
	glDisableVertexAttribArray(attPosition);
	glDisableVertexAttribArray(attColor);

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


void VertexBuffer::Update(const std::vector<VertexColor>& vert, const std::vector<int>& idx) 
{
	_vert = vert;
	_idx = idx;

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _vert.size() * sizeof(VertexColor), _vert.data(), GL_STATIC_DRAW);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

	// Set up vertex buffer array
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	glEnableVertexAttribArray(attPosition);
	glVertexAttribPointer(attPosition, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor), 0); 

	glEnableVertexAttribArray(attColor);
	int rgbOffset = offsetof(VertexColor, red);
	glVertexAttribPointer(attColor, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor), (GLvoid*)(rgbOffset));


	// Set up index buffer array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _idx.size() * sizeof(int), _idx.data(), GL_STATIC_DRAW);

	auto error = glGetError();
	if (error != GL_NO_ERROR)
		throw new std::runtime_error("VertexBuffer: Cannot create vbo!");

	glBindVertexArray(0);
}

void VertexBuffer::Draw()
{
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);

#ifdef USE_VAO
	glBindVertexArray(_vao);
	glDrawElements(GL_LINE_STRIP, _idx.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
#else
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	// assign the vertex shader attributes:   Attribute 0: Position
	glEnableVertexAttribArray(attPosition);
	glVertexAttribPointer(attPosition, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor), 0);

	// assign the vertex shader attributes:   Attribute 1: Color
	glEnableVertexAttribArray(attColor);
	glVertexAttribPointer(attColor, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor), (GLvoid*)(offsetof(VertexColor, red)));

	glDrawElements(GL_LINE_STRIP, _idx.size(), GL_UNSIGNED_INT, nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

	glDisable(GL_PRIMITIVE_RESTART);
}

