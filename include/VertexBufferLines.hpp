#pragma once

#include "VertexBufferBase.hpp"

struct VertexColor
{
	Vec3 pos;
	Color col;
};

class VertexBufferLines : public VertexBufferBase<VertexColor>
{
public:
	VertexBufferLines(int lineWidth = 1, GLuint bufferMode = GL_STATIC_DRAW)
		: VertexBufferBase(bufferMode)
		, _lineWidth(lineWidth)
	{
		DefineAttributes({ 
			{ attPosition, 3, GL_FLOAT, 0 },
			{ attColor,    4, GL_FLOAT, offsetof(VertexColor, col) }
		});
	}

	void OnBeforeDraw() override
	{
//		glLineWidth((GLfloat)_lineWidth);
	}

protected:
	virtual const char* GetVertexShaderSource() const override
	{
		return
			"#version 440 core\n"
			"uniform mat4 projMat;\n"
			"uniform mat4 viewMat;\n"
			"layout(location = 0) in vec3 position;\n"
			"layout(location = 1) in vec4 color;\n"
			"out vec4 vertexColor;\n"
			"void main()\n"
			"{\n"
			"	gl_Position =  projMat * vec4(position, 1);\n"
			"	vertexColor = color;\n"
			"	gl_PointSize = 3;\n"	// point size is only used for the velocity curve
			"}\n";
	}

	virtual const char* GetFragmentShaderSource() const override
	{
		return
			"#version 440 core\n"
			"out vec4 FragColor;\n"
			"in vec4 vertexColor;\n"
			"void main()\n"
			"{\n"
			"	FragColor = vertexColor;\n"
			"}\n";
	}

private:

	enum AttributeIdx : int
	{
		attPosition = 0,
		attColor = 1
	};

	int _lineWidth;
};
