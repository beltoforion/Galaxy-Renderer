#include "TextBuffer.hpp"
#include "MathHelper.hpp"
#include <sstream>

TextBuffer::TextBuffer()
	: _textureData()
	, _pSmallFont(nullptr)
	, _pFont(nullptr)
	, _pFontCaption(nullptr)
{}


TextBuffer::~TextBuffer()
{
	Clear();
}

const char* TextBuffer::GetVertexShaderSource() const
{
	return
		"attribute vec3 vPosition;\n"
		"attribute vec2 vTextureCoord;\n"
		"varying  vec2 vTexCoord;\n"
		"uniform mat4 projMat;\n"
		"void main(void)\n"
		"{\n"
		"	vTexCoord = vTextureCoord;\n"
		"	gl_Position = projMat * vec4(vPosition, 1.0);\n"
		"}\n";
}

const char* TextBuffer::GetFragmentShaderSource() const
{
	return
		"#version 330 core\n"
		"in vec4 vertexColor;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = vertexColor;\n"
		"}\n";
}

GLuint TextBuffer::CreateShader(GLenum shaderType, const char** shaderSource)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, shaderSource, nullptr);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

		std::string msg(infoLog.data());

		// We don't need the shader anymore.
		glDeleteShader(shader);

		throw std::runtime_error(std::string("VertecBuffer: Shader compilation failed: ") + msg);
	}

	return shader;
}

void TextBuffer::Initialize()
{
	_pSmallFont = TTF_OpenFont("consola.ttf", 14);
	if (_pSmallFont == nullptr)
		throw std::runtime_error(TTF_GetError());

	_pFont = TTF_OpenFont("arial.ttf", 18);
	if (_pFont == nullptr)
		throw std::runtime_error(TTF_GetError());

	_pFontCaption = TTF_OpenFont("arial.ttf", 40);
	if (_pFontCaption == nullptr)
		throw std::runtime_error(TTF_GetError());

	glGenBuffers(1, &_vbo);

	const char* srcVertex = GetVertexShaderSource();
	_vertexShader = CreateShader(GL_VERTEX_SHADER, &srcVertex);

	const char* srcFragment = GetFragmentShaderSource();
	_fragmentShader = CreateShader(GL_FRAGMENT_SHADER, &srcFragment);

	_shaderProgram = glCreateProgram();
	glAttachShader(_shaderProgram, _vertexShader);
	glAttachShader(_shaderProgram, _fragmentShader);
	glLinkProgram(_shaderProgram);

	GLint isLinked = 0;
	glGetProgramiv(_shaderProgram, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(_shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(_shaderProgram, maxLength, &maxLength, &infoLog[0]);

		// clean up
		glDeleteProgram(_shaderProgram);
		glDeleteShader(_vertexShader);
		glDeleteShader(_fragmentShader);

		throw std::runtime_error("VertexBuffer: shader program linking failed!");
	}

	// Always detach shaders after a successful link.
	glDetachShader(_shaderProgram, _vertexShader);
	glDetachShader(_shaderProgram, _fragmentShader);
}

float TextBuffer::GetFontSize(int idxFont) const
{
	return (float)TTF_FontHeight(_pFont);
}

TTF_Font* TextBuffer::GetFont(int idxFont) const
{
	switch (idxFont)
	{
	case 1:		return _pFont;
	case 0:		return _pFontCaption;
	default:	return _pSmallFont;
	}
}

void TextBuffer::Clear()
{
	for (auto td : _textureData)
	{
		SDL_FreeSurface(td.surface);
	}

	glDeleteTextures(_textureId.size(), _textureId.data());

	_textureData.clear();
	_textureId.clear();

	if (_vbo != 0)
		glDeleteBuffers(1, &_vbo);
}

void TextBuffer::Draw(int width, int height, glm::mat4& matView, glm::mat4& matProjection)
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width, height, 0, 0, 1);

	for (int i = 0; i < _textureId.size(); ++i)
	{
		auto td = _textureData[i];
		glBindTexture(GL_TEXTURE_2D, _textureId[i]);

		GLfloat xp = td.pos.x;
		GLfloat yp = td.pos.y;
		GLfloat w = td.size.x;
		GLfloat h = td.size.y;
		
		glBegin(GL_TRIANGLES);
		glTexCoord2i(0, 0);
		glVertex3f(xp, yp, 0);
		glTexCoord2i(1, 0);
		glVertex3f(xp + w, yp, 0);
		glTexCoord2i(1, 1);
		glVertex3f(xp + w, yp + h, 0);

		glTexCoord2i(0, 0);
		glVertex3f(xp, yp, 0);
		glTexCoord2i(0, 1);
		glVertex3f(xp, yp + h, 0);
		glTexCoord2i(1, 1);
		glVertex3f(xp + w, yp + h, 0);
		glEnd();
	}

	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

/*
	if (pFont == nullptr)
		throw std::runtime_error("TextOut failed: font is null!");

	if (fmt == nullptr)
		throw std::runtime_error("TextOut failed: bad format string!");

	char text[256];
	va_list ap;

	va_start(ap, fmt);

	vsprintf(text, fmt, ap);
	va_end(ap);

	auto* pSurface = TTF_RenderText_Blended(pFont, text, { 255, 255, 255 });
	if (pSurface == nullptr)
		return;

	GLuint texId;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	SDL_Surface* s = nullptr;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// It seems textures must be powers of 2 in dimension: 
	// https://stackoverflow.com/questions/30016083/sdl2-opengl-sdl2-ttf-displaying-text
	// Create a surface to the correct size in RGB format, and copy the old image
	int w = MathHelper::PowerTwoFloor(pSurface->w) << 1;
	int h = MathHelper::PowerTwoFloor(pSurface->h) << 1;
	s = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_BlitSurface(pSurface, nullptr, s, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);


	glMatrixMode(GL_PROJECTION);
	if (coords == TextCoords::Model)
	{
		auto pos = GetWindowPos((GLfloat)x, (GLfloat)y, 0);
		x = pos.x;
		y = pos.y;
	}

	GLfloat xp = x;
	GLfloat yp = y;

	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, _width, _height, 0, 0, 1);

	// make a rectangle
	glBegin(GL_TRIANGLES);
	glTexCoord2i(0, 0);
	glVertex3f(xp, yp, 0);
	glTexCoord2i(1, 0);
	glVertex3f(xp + w, yp, 0);
	glTexCoord2i(1, 1);
	glVertex3f(xp + w, yp + h, 0);

	glTexCoord2i(0, 0);
	glVertex3f(xp, yp, 0);
	glTexCoord2i(0, 1);
	glVertex3f(xp, yp + h, 0);
	glTexCoord2i(1, 1);
	glVertex3f(xp + w, yp + h, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	// cleanup
	if (s != nullptr)
		SDL_FreeSurface(s);

	if (pSurface != nullptr)
		SDL_FreeSurface(pSurface);

	glDeleteTextures(1, &texId);
	glPopMatrix();
*/
}

void TextBuffer::CreateBuffer() noexcept(false)
{
	_textureId.resize(_textureData.size());

	glGenTextures(_textureData.size(), _textureId.data());
	for (int i = 0; i < _textureId.size(); ++i)
	{
		glBindTexture(GL_TEXTURE_2D, _textureId[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		auto td = _textureData[i];
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, td.size.x, td.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, td.surface->pixels);
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
/*
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _vert.size() * sizeof(VertexTexture), _vert.data(), GL_STATIC_DRAW);

	// Set up vertex buffer array
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	auto errc = glGetError();
	if (errc != GL_NO_ERROR)
	{
		std::stringstream ss;
		ss << "VertexBuffer: Cannot create vbo! (Error 0x" << std::hex << errc << ")" << std::endl;
		throw std::runtime_error(ss.str());
	}

	glBindVertexArray(0);
*/
}


void TextBuffer::AddText(int idxFont, Vec2D pos, const char* fmt, ...)
{
	if (fmt == nullptr)
		throw std::runtime_error("TextBuffer::AddText failed: bad format string!");

	TTF_Font* pFont = GetFont(idxFont);

	char text[256];
	va_list ap;

	va_start(ap, fmt);

	vsprintf(text, fmt, ap);
	va_end(ap);

	auto* pSurface = TTF_RenderText_Blended(pFont, text, { 255, 255, 255 });
	if (pSurface == nullptr)
		throw std::runtime_error("TextBuffer::AddText: TTF_RenderText_Blended failed!");

	SDL_Surface* s = nullptr;

	// It seems textures must be powers of 2 in dimension: 
	// https://stackoverflow.com/questions/30016083/sdl2-opengl-sdl2-ttf-displaying-text
	// Create a surface to the correct size in RGB format, and copy the old image
	int w = MathHelper::PowerTwoFloor(pSurface->w) << 1;
	int h = MathHelper::PowerTwoFloor(pSurface->h) << 1;

	s = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_BlitSurface(pSurface, nullptr, s, nullptr);

	_textureData.push_back({ s, pos, {(float)w, (float)h} });

	if (pSurface != nullptr)
		SDL_FreeSurface(pSurface);
}