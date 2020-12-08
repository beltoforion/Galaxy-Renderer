#pragma once

#include "VertexBufferBase.hpp"


class VertexBufferLines : VertexBufferBase
{
public:
	VertexBufferLines(int lineWidth = 1)
		: VertexBufferBase()
		, _lineWidth(lineWidth)
	{}

protected:
	virtual const char* GetVertexShaderSource() const override
	{
		return
			"#version 330 core\n"
			"uniform mat4 projMat;\n"
			"uniform mat4 viewMat;\n"
			"layout(location = 0) in vec3 position;\n"
			"layout(location = 1) in vec4 color;\n"
			"out vec4 vertexColor;\n"
			"void main()\n"
			"{\n"
			"	gl_Position =  projMat * vec4(position, 1);\n"
			"	vertexColor = color;\n"
			"}\n";
	}

	virtual const char* GetFragmentShaderSource() const override
	{
		return
			"#version 330 core\n"
			"in vec4 vertexColor;\n"
			"void main()\n"
			"{\n"
			"	gl_FragColor = vertexColor;\n"
			"}\n";
	}

private:
	int _lineWidth;

};
