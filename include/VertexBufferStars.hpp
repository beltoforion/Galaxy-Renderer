#pragma once

#include "VertexBufferBase.hpp"


class VertexBufferStars : public VertexBufferBase
{
public:
	VertexBufferStars()
		: VertexBufferBase()
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

	virtual void OnSetupAttribArray() const override 
	{
		glEnableVertexAttribArray(attPosition);
		glVertexAttribPointer(attPosition, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor), 0);

		glEnableVertexAttribArray(attColor);
		uint64_t rgbOffset = offsetof(VertexColor, red);
		glVertexAttribPointer(attColor, 4, GL_FLOAT, GL_FALSE, sizeof(VertexColor), (GLvoid*)(rgbOffset));
	}

	virtual void OnReleaseAttribArray() const override
	{
		glDisableVertexAttribArray(attPosition);
		glDisableVertexAttribArray(attColor);
	}

private:

	enum AttributeIdx : int
	{
		attPosition = 0,
		attColor = 1
	};
};
