#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GalaxyWnd.hpp"

int main(int argc, char** argv)
{
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