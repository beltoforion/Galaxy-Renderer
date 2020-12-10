#include "TextBuffer.hpp"
#include "MathHelper.hpp"


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

void TextBuffer::Initialize()
{
	// Font initialization
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
		glDeleteTextures(1, &td.texId);
	}

	_textureData.clear();
}

void TextBuffer::Draw(glm::mat4& matView, glm::mat4& matProjection)
{

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

	_textureData.push_back({ s, texId, pos, {(float)w, (float)h} });

	if (pSurface != nullptr)
		SDL_FreeSurface(pSurface);
}