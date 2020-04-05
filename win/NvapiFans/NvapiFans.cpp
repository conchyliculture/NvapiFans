#include <iostream>
#include "cxxopts.hpp"
#include "nv.h"
#include "NvapiFans.h"

int main(int argc, char* argv[])
{

	cxxopts::Options options(argv[0], "Tool to monitor Nvdia GPU fan speeds.\n");


	options
		.add_options()
		("h,help", "Print help")
		("l,list", "List GPUs")
		("g,gpu", "Set target GPU id (see --list)", cxxopts::value<int>()->default_value("0"))
		("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
		;

	auto result = options.parse(argc, argv);

	NvApiClient api;

	if (result.count("list"))
	{
		std::string version;
		bool res;
		res = api.showVersion(version);
		if (!res) {
			printf("Failed to get Nvapi version\n");
			return res;
		}

		std::cout << "Nvapi version:" << version << std::endl;
		std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu;
		res = api.getGPUHandles(list_gpu);
		if (!res) {
			printf("Failed to list GPUs\n");
			return res;
		}

		if (list_gpu.size() == 0) {
			std::cout << "Could not detect any NVidia GPU." << std::endl;
			return 0;
		}

		std::cout << "Found " << list_gpu.size() << "NVidia GPUs." << std::endl;

		int index = 0;
		for (NV_PHYSICAL_GPU_HANDLE gpu : list_gpu) {
			std::string gpu_name;
			res = api.getGPUFullname(gpu, gpu_name);
			
			int speedCmd = 0;
			speedCmd = api.getExternalFanSpeedPercent(gpu);

			int actualSpeedRPM1 = 0;
			actualSpeedRPM1 = api.getExternalFanSpeedRPM(gpu, 1);
			int actualSpeedRPM2 = 0;
			actualSpeedRPM2 = api.getExternalFanSpeedRPM(gpu, 2);

			std::cout << "- "<< gpu_name << " [" << index << "] " << std::endl;
			std::cout << "  * External fan speed was set at  " << speedCmd << "%" << std::endl;
			std::cout << "  * Actual External fan1 Speed:" << actualSpeedRPM1 << " RPM" << std::endl;
			std::cout << "  * Actual External fan2 Speed:" << actualSpeedRPM2 << " RPM" << std::endl;
			index += 1;
		}
		exit(res);
	}

	if (result.count("help"))
	{
		std::cout << options.help() << std::endl;
		exit(0);
	}



	return 0;
}
