#include "TextBuffer.hpp"
#include "MathHelper.hpp"
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

TextBuffer::TextBuffer()
	: _textureData()
	, _pSmallFont(nullptr)
	, _pFont(nullptr)
	, _pFontCaption(nullptr)
	, _vbo(0)
	, _updating(false)
	, _shaderProgram(0)
{}

TextBuffer::~TextBuffer()
{
	Clear();
}

const char* TextBuffer::GetVertexShaderSource() const
{
	return
		"#version 330 core\n"
		"layout(location = 0) in vec3 posVert;\n"
		"layout(location = 1) in vec2 posTex;\n"
		"uniform mat4 projMat;\n"
		"out vec2 texCoord;\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"	gl_Position =  projMat * vec4(posVert, 1);\n"
		"	texCoord = vec2(posTex.x, posTex.y);\n"
		"	color = vec4(1,1,1,1);\n"
		"}\n";
}

const char* TextBuffer::GetFragmentShaderSource() const
{
	return
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 color;\n"
		"in vec2 texCoord;\n"
		"uniform sampler2D texture1;"
		"void main()\n"
		"{\n"
		"	FragColor = texture(texture1, texCoord);\n"
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
	if (!TTF_WasInit())
		TTF_Init();

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
	GLuint vertexShader = CreateShader(GL_VERTEX_SHADER, &srcVertex);

	const char* srcFragment = GetFragmentShaderSource();
	GLuint fragmentShader = CreateShader(GL_FRAGMENT_SHADER, &srcFragment);

	_shaderProgram = glCreateProgram();
	glAttachShader(_shaderProgram, vertexShader);
	glAttachShader(_shaderProgram, fragmentShader);
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
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		throw std::runtime_error("VertexBuffer: shader program linking failed!");
	}

	// Always detach shaders after a successful link.
	glDetachShader(_shaderProgram, vertexShader);
	glDetachShader(_shaderProgram, fragmentShader);
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
		glDeleteTextures(1, &td.id);
	}

	_textureData.clear();

	if (_vbo != 0)
		glDeleteBuffers(1, &_vbo);
}

void TextBuffer::Draw(int width, int height, glm::mat4& matView, glm::mat4& matProjection)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glEnable(GL_BLEND);

	glUseProgram(_shaderProgram);

	GLuint vbo, ebo, vao;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	for (int i = 0; i < _textureData.size(); ++i)
	{
		const auto &td = _textureData[i];

		GLfloat x = td.pos.x;
		GLfloat y = td.pos.y;
		GLfloat w = td.size.x;
		GLfloat h = td.size.y;

		VertexTexture vertexData[6]
		{
			{ x,     y,     0, 0, 0 },
			{ x + w, y,     0, 1, 0 },
			{ x + w, y + h, 0, 1, 1},
			{ x,     y + h, 0, 0, 1},
			{ x + w, y + h, 0, 1, 1}
		};

		GLuint indexData[] =
		{
			0, 1, 2,	// first triangle
			0, 3, 4     // second triangle
		};

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

		// Position attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), 0);

		// texture coord attribute
		glEnableVertexAttribArray(1);
		auto offset = offsetof(VertexTexture, tx);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexture), (GLvoid*)offset);

		glBindTexture(GL_TEXTURE_2D, td.id);

		GLuint projMatIdx = glGetUniformLocation(_shaderProgram, "projMat");
		glm::mat4 projection = glm::ortho<float>(0, width, height, 0, 0, 1);
		glUniformMatrix4fv(projMatIdx, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}

	glUseProgram(0);

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	glDisable(GL_BLEND);
	glPopMatrix();
}

void TextBuffer::BeginUpdate()
{
	if (_updating)
		throw std::runtime_error("TextBuffer::BeginUpdate: An update is already in Progress!");

	_updating = true;
	Clear();
}

void TextBuffer::EndUpdate()
{
	if (!_updating)
		throw std::runtime_error("TextBuffer::EndUpdate: No update in progress!");

	_updating = false; 
	CreateBuffer();
}

void TextBuffer::CheckError() const 
{
	auto errc = glGetError();
	if (errc != GL_NO_ERROR)
	{
		std::stringstream ss;
		ss << "GLError: (Error 0x" << std::hex << errc << ")" << std::endl;
		throw std::runtime_error(ss.str());
	}

}
void TextBuffer::CreateBuffer()
{
	for (int i = 0; i < _textureData.size(); ++i)
	{
		auto& td = _textureData[i];
		glGenTextures(1, &td.id);
		glBindTexture(GL_TEXTURE_2D, td.id);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, td.size.x, td.size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, td.surface->pixels);
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
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