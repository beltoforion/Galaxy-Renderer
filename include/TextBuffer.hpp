#pragma once

#include <vector>
#include <stdexcept>

#include <gl/glew.h>
#include <glm/glm.hpp>

#include <SDL_ttf.h>

#include "Star.hpp"


class TextBuffer final
{
public:
	TextBuffer();
	~TextBuffer();

	void Initialize();
	void AddText(int idxFont, Vec2D pos, const char* fmt, ...);
	void Draw(glm::mat4& matView, glm::mat4& matProjection);

	float GetFontSize(int idxFont) const;
	void Clear();

private:
	
	struct TextureData
	{
		SDL_Surface* surface;
		GLuint texId;
		Vec2D pos;
		Vec2D size;
	};

	std::vector<TextureData> _textureData;

	TTF_Font* _pSmallFont;
	TTF_Font* _pFont;
	TTF_Font* _pFontCaption;

	TTF_Font* GetFont(int idxFont) const;

};
