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
	void Draw(int width, int height, glm::mat4& matView, glm::mat4& matProjection);
	float GetFontSize(int idxFont) const;

	void BeginUpdate();
	void EndUpdate();

private:
	
	struct TextureData
	{
		SDL_Surface* surface;
		Vec2D pos;
		Vec2D size;
		GLuint id;
	};

	struct VertexTexture
	{
		float x, y, z;
		float tx, ty;
	};

	enum AttributeIdx : int
	{
		attVertexPosition = 0,
		attTexturePosition = 1
	};

	GLuint _vbo;

	bool _updating;

	std::vector<TextureData> _textureData;
	std::vector<VertexTexture> _vert;
	std::vector<int> _idx;

	GLuint _shaderProgram;

	TTF_Font* _pSmallFont;
	TTF_Font* _pFont;
	TTF_Font* _pFontCaption;

	TTF_Font* GetFont(int idxFont) const;
	const char* GetVertexShaderSource() const;
	const char* GetFragmentShaderSource() const;
	GLuint CreateShader(GLenum shaderType, const char** shaderSource);
	void Clear();
};
