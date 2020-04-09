#pragma once
bool showGPUInfos(const NvApiClient& api, NV_PHYSICAL_GPU_HANDLE gpu);
bool showAllGPUsInfos(const NvApiClient& api, int gpuId);
bool validateGPUId(const std::vector<NV_PHYSICAL_GPU_HANDLE>& list_gpu, int gpuId);
bool setExternalFanSpeed(const NvApiClient& api, int gpuId, int percent);
