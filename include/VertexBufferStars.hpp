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
	VertexBufferStars(GLuint blendEquation, GLuint blendFunc)
		: VertexBufferBase(GL_STATIC_DRAW)
		, _pertN(0)
		, _dustSize(0)
		, _pertAmp(0)
		, _time(0)
		, _blendFunc(blendFunc)
		, _blendEquation(blendEquation)
	{
		DefineAttributes({
			{ attTheta0,        1, GL_FLOAT, offsetof(Star, theta0) },
			{ attVelTheta,      1, GL_FLOAT, offsetof(Star, velTheta) },
			{ attTiltAngle,     1, GL_FLOAT, offsetof(Star, tiltAngle) },
			{ attSemiMajorAxis, 1, GL_FLOAT, offsetof(Star, a) },
			{ attSemiMinorAxis, 1, GL_FLOAT, offsetof(Star, b) },
			{ attTemperature,   1, GL_FLOAT, offsetof(Star, temp) },
			{ attMagnitude,     1, GL_FLOAT, offsetof(Star, mag) },
			{ attType,          1, GL_INT, offsetof(Star, type) },
			{ attColor,         4, GL_FLOAT, offsetof(VertexStar, col) }
		});
	}

	void UpdateShaderVariables(float time, int num, float amp, int dustSize, int displayFeatures)
	{
		_pertN = num;
		_pertAmp = amp;
		_time = time;
		_dustSize = dustSize;
		_displayFeatures = displayFeatures;
	}

	virtual void Draw(glm::mat4& matView, glm::mat4& matProjection)
	{
		CHECK_GL_ERROR
		glUseProgram(GetShaderProgramm());

		GLuint viewMatIdx = glGetUniformLocation(GetShaderProgramm(), "viewMat");
		glUniformMatrix4fv(viewMatIdx, 1, GL_FALSE, glm::value_ptr(matView));

		GLuint projMatIdx = glGetUniformLocation(GetShaderProgramm(), "projMat");
		glUniformMatrix4fv(projMatIdx, 1, GL_FALSE, glm::value_ptr(matProjection));

		OnSetCustomShaderVariables();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, _blendFunc);
		glBlendEquation(_blendEquation);
		glEnable(GL_PROGRAM_POINT_SIZE);
		OnBeforeDraw();

		glBindVertexArray(GetVertexArrayObject());
		glDrawElements(GetPrimitiveType(), GetArrayElementCount(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		glDisable(GL_PROGRAM_POINT_SIZE);
		glDisable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);

		glUseProgram(0);
		CHECK_GL_ERROR
	}

protected:

	virtual const char* GetVertexShaderSource() const override
	{
		return
			"#version 330 core\n"
			"#define DEG_TO_RAD 0.01745329251\n"
			"\n"
			"uniform mat4 projMat;\n"
			"uniform mat4 viewMat;\n"
			"uniform int pertN;\n"
			"uniform int dustSize;\n"
			"uniform int displayFeatures;\n"
			"uniform float pertAmp;\n"
			"uniform float time;\n"
//			"uniform float DEG_TO_RAD = 0.01745329251;\n"
			"\n"
			"layout(location = 0) in float theta0;\n"
			"layout(location = 1) in float velTheta;\n"
			"layout(location = 2) in float tiltAngle;\n"
			"layout(location = 3) in float a;\n"
			"layout(location = 4) in float b;\n"
			"layout(location = 5) in float temp;\n"
			"layout(location = 6) in float mag;\n"
			"layout(location = 7) in int type;\n"
			"layout(location = 8) in vec4 color;\n"
			"\n"
			"out vec4 vertexColor;\n"
			"flat out int vertexType;\n"
			"flat out int features;\n"
			"\n"
			"vec2 calcPos(float a, float b, float theta, float velTheta, float time, float tiltAngle) {\n"
			"	float thetaActual = theta + velTheta * time;\n"
			"	float beta = -tiltAngle;\n"
			"	float alpha = thetaActual * DEG_TO_RAD;\n"
			"	float cosalpha = cos(alpha);\n"
			"	float sinalpha = sin(alpha);\n"
			"	float cosbeta = cos(beta);\n"
			"	float sinbeta = sin(beta);\n"
			"	vec2 center = vec2(0,0);\n"	
			"	vec2 ps = vec2(center.x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta),\n"
			"			       center.y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta));\n"
			"	if (pertAmp > 0.0 && pertN > 0) {\n"
			"		ps.x += (a / pertAmp) * sin(alpha * 2.0 * pertN);\n"
			"		ps.y += (a / pertAmp) * cos(alpha * 2.0 * pertN);\n"
			"	}\n"
			"	return ps;\n"
			"}\n"
			"\n"
			"void main()\n"
			"{\n"
			"	vec2 ps = calcPos(a, b, theta0, velTheta, time, tiltAngle);"	
			"\n"
			"	if (type==0) {\n"
			"		gl_PointSize = mag * 4.0;\n"
			"	    vertexColor = color * mag ;\n"
			"	} else if (type==1) {\n"
			"		gl_PointSize = mag * 5.0 * float(dustSize);\n"
			"	    vertexColor = color * mag;\n"
			"	} else if (type==2) {\n"
			"		gl_PointSize = mag * 2.0 * float(dustSize);\n"
			"	    vertexColor = color * mag;\n"
			"	} else if (type==3) {\n"
			"		vec2 ps2 = calcPos(a + 1000.0, b, theta0, velTheta, time, tiltAngle);\n"
			"		float dst = distance(ps, ps2);\n"
			"		float size = ((1000.0 - dst) / 10) - 50;\n"
			"		gl_PointSize = size;\n"
			"	    vertexColor = color * mag * vec4(2, 0.5, 0.5, 1);\n"
			"	} else if (type==4) {\n"
			"		vec2 ps2 = calcPos(a + 1000.0, b, theta0, velTheta, time, tiltAngle);\n"
			"		float dst = distance(ps, ps2);\n"
			"		float size = ((1000.0 - dst) / 10.0) - 50.0;\n"
			"		gl_PointSize = size/10.0;\n"
			"	    vertexColor = vec4(1,1,1,1);\n"
			"   }\n"
			"	gl_Position =  projMat * vec4(ps, 0, 1);\n"
			"   gl_PointSize = max(gl_PointSize, 0.0);\n"
			"	vertexType = type;\n"
			"	features = displayFeatures;\n"
			"}\n";
	}

	virtual const char* GetFragmentShaderSource() const override
	{
		return
			"#version 330 core\n"
			"in vec4 vertexColor;\n"
			"flat in int vertexType;\n"
			"flat in int features;\n"
			"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"	if (vertexType==0) {\n"
			"		if ( (features & 1) ==0)\n"
			"			discard;\n"
			"		vec2 circCoord = 2.0 * gl_PointCoord - 1.0;\n"
			"		float alpha =1-length(circCoord);\n"
			"		FragColor = vec4(vertexColor.xyz, alpha);\n"
			"	} else if (vertexType==1) {\n"
			"		if ( (features & 2) ==0)\n"
			"			discard;\n"
			"		vec2 circCoord = 2.0 * gl_PointCoord - 1.0;\n"
			"		float alpha = 0.05 * (1-length(circCoord));\n"
			"		FragColor = vec4(vertexColor.xyz, alpha);\n"
			"	} else if (vertexType==2) {\n"
			"		if ( (features & 4) ==0)\n"
			"			discard;\n"	
			"		vec2 circCoord = 2.0 * gl_PointCoord - 1.0;\n"
			"		float alpha = 0.07 * (1-length(circCoord));\n"
			"		FragColor = vec4(vertexColor.xyz, alpha);\n"
			"	} else if (vertexType==3) {\n"
			"		if ((features & 8) == 0)\n"
			"			discard;\n"
			"		vec2 circCoord = 2.0 * gl_PointCoord - 1.0;\n"
			"		float alpha = 1-length(circCoord);\n"
			"		FragColor = vec4(vertexColor.xyz, alpha);\n"
			"	} else if (vertexType==4) {\n"
			"		if ((features & 8)== 0)\n"
			"			discard;\n"
			"		vec2 circCoord = 2.0 * gl_PointCoord - 1.0;\n"
			"		float alpha = 1-length(circCoord);\n"
			"		FragColor = vec4(vertexColor.xyz, alpha);\n"
			"   }\n"
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

		GLuint varDisplayFeatures = glGetUniformLocation(GetShaderProgramm(), "displayFeatures");
		glUniform1i(varDisplayFeatures, _displayFeatures);
	}


private:

	enum AttributeIdx : int
	{
		attTheta0 = 0,
		attVelTheta,
		attTiltAngle,
		attSemiMajorAxis,
		attSemiMinorAxis,
		attTemperature,
		attMagnitude,
		attType,
		attColor
	};

	// parameters for density wave computation
	int _pertN;
	int _dustSize;
	float _pertAmp;
	float _time;
	GLuint _blendFunc;
	GLuint _blendEquation;
	int _displayFeatures;
};
