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
		: VertexBufferBase(GL_STATIC_DRAW)
		, _pertN(0)
		, _dustSize(0)
		, _pertAmp(0)
		, _time(0)
	{
		DefineAttributes({
			{ attPosition,      2, GL_FLOAT, 0 },
			{ attVelocity,      2, GL_FLOAT, offsetof(Star, vel) },
			{ attTheta0,        1, GL_FLOAT, offsetof(Star, theta0) },
			{ attVelTheta,      1, GL_FLOAT, offsetof(Star, velTheta) },
			{ attTiltAngle,     1, GL_FLOAT, offsetof(Star, tiltAngle) },
			{ attSemiMajorAxis, 1, GL_FLOAT, offsetof(Star, a) },
			{ attSemiMinorAxis, 1, GL_FLOAT, offsetof(Star, b) },
			{ attCenter,        2, GL_FLOAT, offsetof(Star, center) },
			{ attTemperature,   1, GL_FLOAT, offsetof(Star, temp) },
			{ attMagnitude,     1, GL_FLOAT, offsetof(Star, mag) },
			{ attType,          1, GL_INT, offsetof(Star, type) },
			{ attColor,         4, GL_FLOAT, offsetof(VertexStar, col) }
		});
	}

	void UpdateShaderVariables(float time, int num, float amp, int dustSize)
	{
		_pertN = num;
		_pertAmp = amp;
		_time = time;
		_dustSize = dustSize;
	}

	virtual void Draw(glm::mat4& matView, glm::mat4& matProjection)
	{
		glUseProgram(GetShaderProgramm());

		GLuint viewMatIdx = glGetUniformLocation(GetShaderProgramm(), "viewMat");
		glUniformMatrix4fv(viewMatIdx, 1, GL_FALSE, glm::value_ptr(matView));

		GLuint projMatIdx = glGetUniformLocation(GetShaderProgramm(), "projMat");
		glUniformMatrix4fv(projMatIdx, 1, GL_FALSE, glm::value_ptr(matProjection));

		OnSetCustomShaderVariables();

		glEnable(GL_BLEND);
		glEnable(GL_PROGRAM_POINT_SIZE);
//		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glEnable(GL_POINT_SPRITE);
		OnBeforeDraw();

		glBindVertexArray(GetVertexArrayObject());
		glDrawElements(GetPrimitiveType(), GetArrayElementCount(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		glDisable(GL_POINT_SPRITE);
//		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDisable(GL_PROGRAM_POINT_SIZE);
		glDisable(GL_BLEND);

		glUseProgram(0);
	}

protected:

	virtual const char* GetVertexShaderSource() const override
	{
		return
			"#version 440 core\n"
			"\n"
			"uniform mat4 projMat;\n"
			"uniform mat4 viewMat;\n"
			"uniform int pertN;\n"
			"uniform int dustSize;\n"
			"uniform float pertAmp;\n"
			"uniform float time;\n"
			"uniform float DEG_TO_RAD = 0.01745329251;\n"
			"\n"
			"layout(location = 0) in vec2 pos;\n"
			"layout(location = 1) in vec2 vel;\n"
			"layout(location = 2) in float theta0;\n"
			"layout(location = 3) in float velTheta;\n"
			"layout(location = 4) in float tiltAngle;\n"
			"layout(location = 5) in float a;\n"
			"layout(location = 6) in float b;\n"
			"layout(location = 7) in vec2 center;\n"
			"layout(location = 8) in float temp;\n"
			"layout(location = 9) in float mag;\n"
			"layout(location = 10) in int type;\n"
			"layout(location = 11) in vec4 color;\n"
			"\n"
			"out vec4 vertexColor;\n"
			"out float vertexType;\n"
			"\n"
			"void main()\n"
			"{\n"
			"	float thetaActual = theta0 + velTheta * time;\n"
			"\n"
			"	float beta = -tiltAngle;\n"
			"	float alpha = thetaActual * DEG_TO_RAD;\n"
			"\n"
			"	float cosalpha = cos(alpha);\n"
			"	float sinalpha = sin(alpha);\n"
			"	float cosbeta = cos(beta);\n"
			"	float sinbeta = sin(beta);\n"
			"\n"
			"	vec2 ps = vec2(center.x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta),\n"
			"			       center.y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta));\n"
			"\n"
			"	if (pertAmp > 0 && pertN > 0) {\n"
			"		ps.x += (a / pertAmp) * sin(alpha * 2 * pertN);\n"
			"		ps.y += (a / pertAmp) * cos(alpha * 2 * pertN);\n"
			"	}\n"
			"\n"
			"	if (type==0)\n"
			"		gl_PointSize = mag * 3;\n"
			"	else"	
			"		gl_PointSize = dustSize * mag * 50;\n"
			"\n"
			"	gl_Position =  projMat * vec4(ps, 0, 1);\n"
			"	vertexColor = color * mag;\n"
			"	vertexType = float(type);\n"
			"}\n";
	}

	virtual const char* GetFragmentShaderSource() const override
	{
		return
			"#version 440 core\n"
			"in vec4 vertexColor;\n"
			"in float vertexType;\n"
			"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"	if (vertexType==0) {\n"
			"		FragColor = vertexColor;\n"
			"	} else {\n"
			"		vec2 circCoord = 2.0 * gl_PointCoord - 1.0;\n"
			"		float alpha = 0.3 * (1-length(circCoord));\n"
			"		FragColor = vec4(vertexColor.xyz, alpha);\n"
			"	}\n"
			"}\n";
	}

	virtual void OnSetCustomShaderVariables() override
	{
		GLuint varDustSize = glGetUniformLocation(GetShaderProgramm(), "dustSize");
		glUniform1i(varDustSize, _dustSize);

		GLuint varPertN = glGetUniformLocation(GetShaderProgramm(), "pertN");
		glUniform1i(varPertN, _pertN);

		GLuint varPertAmp = glGetUniformLocation(GetShaderProgramm(), "pertAmp");
		glUniform1f(varPertAmp, _pertAmp);

		GLuint varTime = glGetUniformLocation(GetShaderProgramm(), "time");
		glUniform1f(varTime, _time);
	}


private:

	enum AttributeIdx : int
	{
		attPosition = 0,
		attVelocity = 1,
		attTheta0 = 2,
		attVelTheta = 3,
		attTiltAngle = 4,
		attSemiMajorAxis = 5,
		attSemiMinorAxis = 6,
		attCenter = 7,
		attTemperature = 8,
		attMagnitude = 9,
		attType = 10,
		attColor = 11
	};

	// parameters for density wave computation
	int _pertN;
	int _dustSize;
	float _pertAmp;
	float _time;
};
