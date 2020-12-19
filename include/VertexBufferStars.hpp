#pragma once

#include "VertexBufferBase.hpp"

struct VertexStar
{
	Star star;
	Color col;
};

class VertexBufferStars : public VertexBufferBase<VertexStar>
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
			"layout(location = 0) in vec2 pos;\n"
			"layout(location = 1) in vec2 vel;\n"
			"layout(location = 2) in float theta;\n"
			"layout(location = 3) in float velTheta;\n"
			"layout(location = 4) in float angle;\n"
			"layout(location = 5) in float a;\n"
			"layout(location = 6) in float b;\n"
			"layout(location = 7) in vec2 center;\n"
			"layout(location = 8) in float temp;\n"
			"layout(location = 9) in float mag;\n"
			"layout(location = 10) in vec4 color;\n"
			"out vec4 vertexColor;\n"
			"void main()\n"
			"{\n"
			"	gl_Position =  projMat * vec4(pos, 0, 1);\n"
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
		glVertexAttribPointer(attPosition, 2, GL_FLOAT, GL_FALSE, sizeof(VertexStar), 0);

		glEnableVertexAttribArray(attVelocity);
		glVertexAttribPointer(attVelocity, 2, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, vel)));

		glEnableVertexAttribArray(attTheta);
		glVertexAttribPointer(attTheta, 1, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, theta)));

		glEnableVertexAttribArray(attVelTheta);
		glVertexAttribPointer(attVelTheta, 1, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, velTheta)));

		glEnableVertexAttribArray(attAngle);
		glVertexAttribPointer(attAngle, 1, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, angle)));

		glEnableVertexAttribArray(attSemiMajorAxis);
		glVertexAttribPointer(attSemiMajorAxis, 1, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, a)));

		glEnableVertexAttribArray(attSemiMinorAxis);
		glVertexAttribPointer(attSemiMinorAxis, 1, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, b)));

		glEnableVertexAttribArray(attCenter);
		glVertexAttribPointer(attCenter, 2, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, center)));

		glEnableVertexAttribArray(attTemperature);
		glVertexAttribPointer(attTemperature, 1, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, temp)));

		glEnableVertexAttribArray(attMagnitude);
		glVertexAttribPointer(attMagnitude, 1, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(Star, mag)));

		glEnableVertexAttribArray(attColor);
		glVertexAttribPointer(attColor, 4, GL_FLOAT, GL_FALSE, sizeof(VertexStar), (GLvoid*)(offsetof(VertexStar, col)));
	}

	virtual void OnReleaseAttribArray() const override
	{
		for (int i = 0; i < attLast; ++i)
		{
			glDisableVertexAttribArray(i);
		}
	}

private:

	enum AttributeIdx : int
	{
		attPosition = 0,
		attVelocity = 1,
		attTheta = 2,
		attVelTheta = 3,
		attAngle = 4,
		attSemiMajorAxis = 5,
		attSemiMinorAxis = 6,
		attCenter = 7,
		attTemperature = 8,
		attMagnitude = 9,
		attColor = 10,
		attLast
	};
};
