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

	/// Parameters of the H2 region shader model: the density wave shape needed
	/// to locate the neighbouring waves plus the ignition tuning values.
	struct H2Params
	{
		float radCore;      ///< radius of the galaxy core (see Galaxy::GetExcentricity)
		float radGalaxy;    ///< radius of the galaxy disc
		float radFarField;  ///< radius beyond which the waves become circular
		float exInner;      ///< excentricity at the core edge
		float exOuter;      ///< excentricity at the disc edge
		float angleOffset;  ///< ellipse tilt per parsec (the spiral twist)
		float sizeMax;      ///< point size of a fully ignited region (px)
		float threshold;    ///< density enhancement rho at which a region ignites
	};
	VertexBufferStars(GLuint blendEquation, GLuint blendFunc)
		: VertexBufferBase(GL_STATIC_DRAW)
		, _pertN(0)
		, _dustSize(0)
		, _pertAmp(0)
		, _time(0)
		, _blendFunc(blendFunc)
		, _blendEquation(blendEquation)
		, _sizeFactor(1)
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

	void UpdateH2Params(const H2Params& params)
	{
		_h2 = params;
	}

	/// Scales all point sizes. Needed to keep the relative sizes of stars and
	/// dust intact when rendering into an offscreen buffer with a resolution
	/// different from the window (i.e. when recording video).
	void SetSizeFactor(float sizeFactor)
	{
		_sizeFactor = sizeFactor;
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
		// Enable point sprite coordinate replacement so gl_PointCoord is provided
		// in the fragment shader. Officially GL_POINT_SPRITE does not exist in
		// core profiles (coord replacement is always on there), but some drivers
		// (e.g. NVIDIA on Linux) still require the enable even on core contexts -
		// without it gl_PointCoord stays (0,0) and the stars become invisible
		// (their alpha turns negative under additive blending). Strict drivers
		// (e.g. WGL under Wine/Windows) instead reject the enum with
		// GL_INVALID_ENUM, which CHECK_GL_ERROR would escalate to a fatal error.
		// So probe once: try the enable and remember whether the driver takes it.
		static const bool usePointSprite = []() {
			while (glGetError() != GL_NO_ERROR) {}  // drain pending errors
			glEnable(GL_POINT_SPRITE);
			return glGetError() == GL_NO_ERROR;
		}();
		if (usePointSprite)
			glEnable(GL_POINT_SPRITE);
		OnBeforeDraw();

		glBindVertexArray(GetVertexArrayObject());
		glDrawElements(GetPrimitiveType(), GetArrayElementCount(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		if (usePointSprite)
			glDisable(GL_POINT_SPRITE);
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
			"uniform float sizeFactor;\n"
			"uniform float radCore;\n"
			"uniform float radGalaxy;\n"
			"uniform float radFarField;\n"
			"uniform float exInner;\n"
			"uniform float exOuter;\n"
			"uniform float angleOffset;\n"
			"uniform float h2SizeMax;\n"
			"uniform float h2Threshold;\n"
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
			"// Excentricity of the density wave at radius r (mirrors Galaxy::GetExcentricity).\n"
			"float excentricity(float r) {\n"
			"	if (r < radCore)\n"
			"		return 1.0 + (r / radCore) * (exInner - 1.0);\n"
			"	else if (r <= radGalaxy)\n"
			"		return exInner + (r - radCore) / (radGalaxy - radCore) * (exOuter - exInner);\n"
			"	else if (r < radFarField)\n"
			"		return exOuter + (r - radGalaxy) / (radFarField - radGalaxy) * (1.0 - exOuter);\n"
			"	else\n"
			"		return 1.0;\n"
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
			"	} else if (type==3 || type==4) {\n"
			"		// Orbit crowding: measure the radial gap to the neighbouring density\n"
			"		// waves at a +/- delta, each with its own excentricity and tilt. The\n"
			"		// parametric angle is shifted by the tilt difference so both points\n"
			"		// lie at the same polar angle; the distance is then the true wave\n"
			"		// spacing. Where waves converge (arm crest) the region ignites.\n"
			"		float delta = 1000.0;\n"
			"		float aI = max(a - delta, 0.0);\n"
			"		float dI = a - aI;\n"
			"		float aO = a + delta;\n"
			"		vec2 psI = calcPos(aI, aI * excentricity(aI), theta0 - (dI * angleOffset) / DEG_TO_RAD, velTheta, time, aI * angleOffset);\n"
			"		vec2 psO = calcPos(aO, aO * excentricity(aO), theta0 + (delta * angleOffset) / DEG_TO_RAD, velTheta, time, aO * angleOffset);\n"
			"		float rho = 0.5 * (dI / max(distance(ps, psI), 1.0) + delta / max(distance(ps, psO), 1.0));\n"
			"		float ignite = smoothstep(h2Threshold, 1.5 * h2Threshold, rho);\n"
			"		if (type==3) {\n"
			"			gl_PointSize = h2SizeMax * ignite;\n"
			"			vertexColor = color * mag * vec4(2.0, 0.5, 0.5, 1.0) * ignite;\n"
			"		} else {\n"
			"			gl_PointSize = h2SizeMax * ignite / 10.0;\n"
			"			vertexColor = vec4(1,1,1,1) * ignite;\n"
			"		}\n"
			"   }\n"
			"	gl_Position =  projMat * vec4(ps, 0, 1);\n"
			"   gl_PointSize = max(gl_PointSize * sizeFactor, 0.0);\n"
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

		GLuint varSizeFactor = glGetUniformLocation(GetShaderProgramm(), "sizeFactor");
		glUniform1f(varSizeFactor, _sizeFactor);

		glUniform1f(glGetUniformLocation(GetShaderProgramm(), "radCore"), _h2.radCore);
		glUniform1f(glGetUniformLocation(GetShaderProgramm(), "radGalaxy"), _h2.radGalaxy);
		glUniform1f(glGetUniformLocation(GetShaderProgramm(), "radFarField"), _h2.radFarField);
		glUniform1f(glGetUniformLocation(GetShaderProgramm(), "exInner"), _h2.exInner);
		glUniform1f(glGetUniformLocation(GetShaderProgramm(), "exOuter"), _h2.exOuter);
		glUniform1f(glGetUniformLocation(GetShaderProgramm(), "angleOffset"), _h2.angleOffset);
		glUniform1f(glGetUniformLocation(GetShaderProgramm(), "h2SizeMax"), _h2.sizeMax);
		glUniform1f(glGetUniformLocation(GetShaderProgramm(), "h2Threshold"), _h2.threshold);
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
	float _sizeFactor;
	H2Params _h2 = {};
};
