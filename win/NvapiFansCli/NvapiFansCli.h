#pragma once
bool showGPUInfos(NvApiClient api, NV_PHYSICAL_GPU_HANDLE gpu);
bool showAllGPUsInfos(NvApiClient api, int gpuId);
bool validateGPUId(std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu, int gpuId);
bool setExternalFanSpeed(NvApiClient api, int gpuId, int percent);
