#include <cstdio>
#include "nv.h"
#include "NvapiFans.h"


int main()
{
	bool res;
	NvApiClient api;

	std::string version;
	res = api.showVersion(version);
	if (!res) {
		printf("Failed to get version\n");
	}
	std::cout << "Nvapi version:" << version << std::endl;

	std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu;
	res = api.getGPUHandles(list_gpu);
	if (!res) {
		printf("Failed to list GPUs\n");
		return -1;
	}
	
	std::vector<NV_PHYSICAL_GPU_HANDLE>::iterator ptr;
	for (NV_PHYSICAL_GPU_HANDLE gpu: list_gpu) {
		std::string gpu_name;
		res = api.getGPUFullname(gpu, gpu_name);
		printf("Found physical GPU: %s", gpu_name.c_str() );
		printf("\n");

		byte speed = 0;
		speed = api.getExternalFanSpeedPercent(gpu);
		printf("Old Speed : %d%%\n", speed);

		speed = speed - 10;
		api.setExternalFanSpeedPercent(gpu, speed);

		speed = api.getExternalFanSpeedPercent(gpu);
		printf("New Speed : %d%%\n", speed);
	}

	return 0;
}
