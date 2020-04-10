#pragma warning(disable : 26812)
#include <iostream>
#include "cxxopts.hpp"
#include "NvapiFansLib.h"


bool validateGPUId(const std::vector<NV_PHYSICAL_GPU_HANDLE>& list_gpu, int gpuId) {
	if (gpuId < -1 || gpuId > NVAPI_MAX_PHYSICAL_GPUS) {
		std::cerr << "Invalid gpu id: " << gpuId;
		return false;
	}
	if (list_gpu.size() == 0) {
		std::cout << "Could not detect any NVidia GPU." << std::endl;
		return false;
	}

	if (gpuId > 0 && gpuId >= (int)list_gpu.size()) {
		std::cerr << "GPU id provided is " << gpuId << ". Max GPU id is " << list_gpu.size() - 1 << std::endl;
		return false;
	}
	return true;
}

// Displays information for a specific gpu handle.
bool showGPUInfos(const NvApiClient& api, NV_PHYSICAL_GPU_HANDLE gpu) {
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

std::vector<NV_PHYSICAL_GPU_HANDLE> getAllGPUs(const NvApiClient& api) {
	std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu;
	bool res = api.getGPUHandles(list_gpu);
	if (!res) {
		printf("Failed to list GPUs\n");
	}

	return list_gpu;
}

// Collects the list of GPU handles, and will display informations related to them.
// If gpuId is anything >= 0, will only show info for this one.
bool showAllGPUsInfos(const NvApiClient& api, int gpuId) {
	bool res = true;

	std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu = getAllGPUs(api);

	if (!validateGPUId(list_gpu, gpuId)) {
		return false;
	}

	std::cout << "Found " << list_gpu.size() << " NVidia GPUs." << std::endl;

	int index = 0;
	for (NV_PHYSICAL_GPU_HANDLE gpu : list_gpu) {
		if (gpuId < 0 || index == gpuId) {
			res &= showGPUInfos(api, gpu);
		}
		index += 1;
	}
	return res;
}

bool setExternalFanSpeed(const NvApiClient& api, int gpuId, int percent) {
	bool res;

	if (percent < 0 || percent >100) {
		std::cerr << "Fan speed needs to be between 0 and 100" << std::endl;
		return false;
	}

	std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu = getAllGPUs(api);
	if (!validateGPUId(list_gpu, gpuId)) {
		return false;
	}

	int index = 0;
	for (NV_PHYSICAL_GPU_HANDLE gpu : list_gpu) {
		if (gpuId < 0 || index == gpuId) {
			res &= api.setExternalFanSpeedPercent(gpu, percent);
		}
		index += 1;
	}

	res &= api.setExternalFanSpeedPercent(list_gpu.at(gpuId), percent);
	
	return res;
}

int main(int argc, char* argv[])
{
	bool res = false;
	try
	{
		cxxopts::Options options(argv[0], "Tool to monitor Nvdia GPU fan speeds.\n");

		options
			//.allow_unrecognised_options()
			.add_options()
			("h,help", "Print help")
			("l,list", "List GPUs")
			("e,external", "Set external fan speed (in %)", cxxopts::value<int>())
			("g,gpu", "Set target GPU id (-1 is 'all GPUs')", cxxopts::value<int>())//->default_value("-1")->implicit_value("0"))
			;

		auto result = options.parse(argc, argv);
		int gpuId = 0;

		if (result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}

		NvApiClient api;
		if (result.count("debug")) {
			std::string version;

			res = api.getNvapiVersion(version);
			if (!res) {
				printf("Failed to get Nvapi version\n");
				return res;
			}
			std::cout << "Nvapi version:" << version << std::endl;
		}

		std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu = getAllGPUs(api);
		bool detected = true;
		for (NV_PHYSICAL_GPU_HANDLE gpu : list_gpu) {
			detected &= api.detectI2CDevice(gpu);
		}
		if (!detected) {
			std::cerr << "Failed to detect all GPU I2C devices" << std::endl;
			exit(1);
		}

		if (result.count("gpu")) {
			gpuId = result["gpu"].as<int>();
		}

		if (result.count("external")) {
			res = setExternalFanSpeed(api, gpuId, result["external"].as<int>());
			return res;
		}

		if (result.count("list")) {
			bool res = showAllGPUsInfos(api, gpuId);
			return res;
		};

		// No argument: show info all GPUs
		bool res = showAllGPUsInfos(api, gpuId);
		return res;
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		exit(1);
	}
}
