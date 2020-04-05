#include <iostream>
#include "cxxopts.hpp"
#include "nv.h"
#include "NvapiFans.h"


bool showGPUInfos(NvApiClient api, NV_PHYSICAL_GPU_HANDLE gpu) {
	bool res;
	std::string gpu_name;
	res = api.getGPUFullname(gpu, gpu_name);

	int speedCmd = 0;
	speedCmd = api.getExternalFanSpeedPercent(gpu);

	int actualSpeedRPM1 = 0;
	actualSpeedRPM1 = api.getExternalFanSpeedRPM(gpu, 1);
	int actualSpeedRPM2 = 0;
	actualSpeedRPM2 = api.getExternalFanSpeedRPM(gpu, 2);

	std::cout << "- " << gpu_name << std::endl;
	std::cout << "  * External fan speed was set at " << speedCmd << "%" << std::endl;
	std::cout << "  * Actual External fan1 Speed: " << actualSpeedRPM1 << " RPM" << std::endl;
	std::cout << "  * Actual External fan2 Speed: " << actualSpeedRPM2 << " RPM" << std::endl;

	NV_GPU_THERMAL_SETTINGS infos{};
	infos.version = NV_GPU_THERMAL_SETTINGS_VER_2;
	res |= api.getTemps(gpu, infos);
	for (NvU32 i = 0; i < infos.count; i++) {
		NV_THERMAL_TARGET target = infos.sensor[i].target;
		switch (target) {
		case NVAPI_THERMAL_TARGET_GPU:
			std::cout << "  * GPU temp: " << infos.sensor[i].currentTemp << "C" << std::endl;
		case NVAPI_THERMAL_TARGET_MEMORY:
			std::cout << "  * Memory temp: " << infos.sensor[i].currentTemp << "C" << std::endl; break;
		case NVAPI_THERMAL_TARGET_POWER_SUPPLY:
			std::cout << "  * Power Supply temp: " << infos.sensor[i].currentTemp << "C" << std::endl; break;
		case NVAPI_THERMAL_TARGET_BOARD:
			std::cout << "  * Board temp: " << infos.sensor[i].currentTemp << "C" << std::endl; break;
		case NVAPI_THERMAL_TARGET_VCD_BOARD:
			std::cout << "  * VCD Board temp: " << infos.sensor[i].currentTemp << "C" << std::endl; break;
		case NVAPI_THERMAL_TARGET_UNKNOWN:
			std::cout << "  * Unknown temp: " << infos.sensor[i].currentTemp << "C" << std::endl; break;
		}
	}
	return res;
}

bool showAllGPUsInfos(NvApiClient api, int gpuId) {
	std::string version;
	bool res;
	res = api.showVersion(version);
	if (!res) {
		printf("Failed to get Nvapi version\n");
		return res;
	}

	std::cout << "Nvapi version:" << version << std::endl;
	std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu;
	res |= api.getGPUHandles(list_gpu);
	if (!res) {
		printf("Failed to list GPUs\n");
		return res;
	}

	if (list_gpu.size() == 0) {
		std::cout << "Could not detect any NVidia GPU." << std::endl;
		return 0;
	}

	std::cout << "Found " << list_gpu.size() << " NVidia GPUs." << std::endl;

	int index = 0;
	for (NV_PHYSICAL_GPU_HANDLE gpu : list_gpu) {
		if (gpuId < 0 || index == gpuId) {
			res |= showGPUInfos(api, gpu);
		}
		index += 1;
	}
	return res;
}

int main(int argc, char* argv[])
{
	try
	{
		cxxopts::Options options(argv[0], "Tool to monitor Nvdia GPU fan speeds.\n");

		options
			//.allow_unrecognised_options()
			.add_options()
			("h,help", "Print help")
			("l,list", "List GPUs")
			("g,gpu", "Set target GPU id (-1 is 'all GPUs')", cxxopts::value<int>())//->default_value("-1")->implicit_value("0"))
			;

		auto result = options.parse(argc, argv);
		int gpu = 0;
		
		if (result.count("gpu")) {
			gpu = result["gpu"].as<int>();
		}
		NvApiClient api;

		std::cout << gpu << std::endl;
		if (result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}

		if (result.count("list")) {

		};

		// No argument: list all GPUs

		bool res = showAllGPUsInfos(api, gpu);
		return res;
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		exit(1);
	}
}
