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
	{
		_attributes.push_back({ attPosition,      2, 0 });
		_attributes.push_back({ attVelocity,      2, offsetof(Star, vel) });
		_attributes.push_back({ attTheta,         1, offsetof(Star, theta) });
		_attributes.push_back({ attVelTheta,      1, offsetof(Star, velTheta) });
		_attributes.push_back({ attAngle,         1, offsetof(Star, angle) });
		_attributes.push_back({ attSemiMajorAxis, 1, offsetof(Star, a) });
		_attributes.push_back({ attSemiMinorAxis, 1, offsetof(Star, b) });
		_attributes.push_back({ attCenter,        2, offsetof(Star, center) });
		_attributes.push_back({ attTemperature,   1, offsetof(Star, temp) });
		_attributes.push_back({ attMagnitude,     1, offsetof(Star, mag) });
		_attributes.push_back({ attColor,         4, offsetof(VertexStar, col) });
	}

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
		attColor = 10
	};
};
