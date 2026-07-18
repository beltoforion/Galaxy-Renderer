#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "GalaxyWnd.hpp"

static void PrintUsage()
{
	std::cout
		<< "Usage: galaxy_renderer [options]\n"
		<< "\n"
		<< "Options:\n"
		<< "  --video-size WxH   Resolution of the exported video (default: 3840x2160)\n"
		<< "  --video-fps N      Frame rate of the exported video (default: 60)\n"
		<< "  --help             Show this help\n"
		<< "\n"
		<< "Press [F7] in the application to start/stop the video recording.\n"
		<< "Video export requires ffmpeg to be installed and in the search path.\n";
}

int main(int argc, char** argv)
{
	int videoWidth = 3840;
	int videoHeight = 2160;
	int videoFps = 60;

	for (int i = 1; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "--help") == 0)
		{
			PrintUsage();
			return 0;
		}
		else if (std::strcmp(argv[i], "--video-size") == 0 && i + 1 < argc)
		{
			if (std::sscanf(argv[++i], "%dx%d", &videoWidth, &videoHeight) != 2 || videoWidth < 2 || videoHeight < 2)
			{
				std::cout << "Invalid argument for --video-size, expected something like 3840x2160" << std::endl;
				return 1;
			}
		}
		else if (std::strcmp(argv[i], "--video-fps") == 0 && i + 1 < argc)
		{
			videoFps = std::atoi(argv[++i]);
			if (videoFps < 1)
			{
				std::cout << "Invalid argument for --video-fps" << std::endl;
				return 1;
			}
		}
		else
		{
			std::cout << "Unknown option: " << argv[i] << std::endl;
			PrintUsage();
			return 1;
		}
	}

	try
	{
		GalaxyWnd wndMain;
		wndMain.SetVideoOptions(videoWidth, videoHeight, videoFps);
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

	return 0;
}
