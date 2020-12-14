#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GalaxyWnd.hpp"

/* Test code for shader texture output
GLuint CreateShader(GLenum shaderType, const char** shaderSource)
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


void test()
{
	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	int width = 1500;
	int height = 1000;
	SDL_Window *pSdlWnd = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
	assert(pSdlWnd != nullptr);

	SDL_Renderer *pSdlRenderer = SDL_CreateRenderer(pSdlWnd, -1, SDL_RENDERER_ACCELERATED);
	SDL_GLContext sdcGlContext = SDL_GL_CreateContext(pSdlWnd);

	glewInit();

	//
	// GL initialization
	//

	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, width, height);

#pragma region Shader Creation

	const char* srcVertex = 
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
	GLuint vertexShader = CreateShader(GL_VERTEX_SHADER, &srcVertex);

	const char* srcFragment = 	
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 color;\n"
		"in vec2 texCoord;\n"
		"uniform sampler2D texture1;"
		"void main()\n"
		"{\n"
		"	FragColor = texture(texture1, texCoord);\n"
		"}\n";
	GLuint fragmentShader = CreateShader(GL_FRAGMENT_SHADER, &srcFragment);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	GLint isLinked = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);

		// clean up
		glDeleteProgram(shaderProgram);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		throw std::runtime_error("VertexBuffer: shader program linking failed!");
	}

	// Always detach shaders after a successful link.
	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);

#pragma endregion
		
#pragma region Texture loading

	SDL_Surface* tex;
	tex = SDL_LoadBMP("particle.bmp");
	if (!tex)
		throw std::runtime_error("Can't load star texture (particle.bmp).");

	// get the number of channels in the SDL surface
	GLint  nOfColors = tex->format->BytesPerPixel;
	GLenum texture_format = GL_RGB;

	// Have OpenGL generate a texture object handle for us
	GLuint texStar;
	glGenTextures(1, &texStar);

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, texStar);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, tex->w, tex->h, 0, texture_format, GL_UNSIGNED_BYTE, tex->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

	glDisable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, 0, 1);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	struct VertexTexture
	{
		float x, y, z;
		float tx, ty;
	};

	SDL_Event event;
	while (true)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				exit(0);
				break;
			} // switch event type
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3f(1, 0, 0);

		GLfloat x = 500; // -0.5;
		GLfloat y = 500; // -0.5;
		GLfloat w = 250; // 1;
		GLfloat h = 250; // 1;

#define USE_BUF
#if defined USE_BUF
		glPushMatrix();
		VertexTexture vertexData[5]
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

		GLuint vbo, ebo, vao;
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

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

		glBindTexture(GL_TEXTURE_2D, texStar);

		glUseProgram(shaderProgram);
		GLuint projMatIdx = glGetUniformLocation(shaderProgram, "projMat");
		glm::mat4 projection = glm::ortho<float>(0, width, height, 0, 0, 1);
//		glm::mat4 projection = glm::ortho(-1.0f, 1.0f, 2.0f, -2.0f, -1.0f, 1.0f);
		glUniformMatrix4fv(projMatIdx, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		
		glUseProgram(0);
		
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
		glPopMatrix();
#else
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texStar);
		glBegin(GL_TRIANGLES);
			glTexCoord2f(0, 0);
			glVertex3f(x, y, 0);

			glTexCoord2f(1, 0);
			glVertex3f(x + w, y, 0);

			glTexCoord2f(1, 1);
			glVertex3f(x + w, y + h, 0);

			glTexCoord2f(0, 0);
			glVertex3f(x, y, 0);
			
			glTexCoord2f(0, 1);
			glVertex3f(x, y + h, 0);
			
			glTexCoord2f(1, 1);
			glVertex3f(x + w, y + h, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
#endif

		SDL_GL_SwapWindow(pSdlWnd);
		SDL_Delay(100);
	};
}
*/

int main(int argc, char** argv)
{
	//test();
	//return 0;

	try
	{
		GalaxyWnd wndMain;
		wndMain.Init(1500, 1000, 35000.0, "Rendering a Galaxy with Density Waves");
		wndMain.MainLoop();
	}
	catch (std::exception& exc)
	{
		std::cout << "Fatal error: " << exc.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "Fatal error: unexpected exception" << std::endl;
	}

	return (EXIT_SUCCESS);
}