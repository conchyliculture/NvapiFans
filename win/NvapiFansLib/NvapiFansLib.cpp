// NvapiFansLib.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <math.h>       /* ceil & floor */
#include "NvapiFansLib.h"


static int hexToPercent(byte hex) {
	return (int)floor((hex * 100.0) / 0xFF);
};
static int percentToHex(byte hex) {
	int hex_val = (int)hex;
	return (int)ceil((hex_val * 0xFF) / 100.0);
};

NvApiClient::NvApiClient() {

	#ifdef _WIN64
		hNvapi = LoadLibrary(L"nvapi64.dll");
	#else
		hNvapi = LoadLibrary(L"nvapi.dll");
	#endif

	bool success = false;

	if (hNvapi == nullptr) {
		DWORD dError = GetLastError();
		std::cerr << "Failed to load nvapi.dll, code:"<< dError << std::endl;
	}
	if (hNvapi != NULL) {
		// nvapi_QueryInterface is a function used to retrieve other internal functions in nvapi.dll
		NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress(hNvapi, "nvapi_QueryInterface");
	
		if (NvAPI_QueryInterface != nullptr) {
			// some useful internal functions that aren't exported by nvapi.dll
			// Values are available at https://github.com/tokkenno/nvapi.net/wiki/NvAPI-Functions
			NvAPI_Initialize = (NvAPI_Initialize_t)(*NvAPI_QueryInterface)(0x0150E828);
			NvAPI_EnumPhysicalGPUs = (NvAPI_EnumPhysicalGPUs_t)(*NvAPI_QueryInterface)(0xE5AC921F);
			NvAPI_GetInterfaceVersionString = (NvAPI_GetInterfaceVersionString_t)(*NvAPI_QueryInterface)(0x01053FA5);
			NvAPI_GPU_GetFullName = (NvAPI_GPU_GetFullName_t)(*NvAPI_QueryInterface)(0xCEEE8E9F);
			NvAPI_GetErrorMessage = (NvAPI_GetErrorMessage_t)(*NvAPI_QueryInterface)(0x6C2D048C);
			NvAPI_I2CReadEx = (NvAPI_I2CReadEx_t)(*NvAPI_QueryInterface)(0x4D7B0709);
			NvAPI_I2CWriteEx = (NvAPI_I2CWriteEx_t)(*NvAPI_QueryInterface)(0x283AC65A);
			NvAPI_GPU_GetThermalSettings = (NvAPI_GPU_GetThermalSettings_t)(*NvAPI_QueryInterface)(0xE3640A56);
			NvAPI_GPU_GetDynamicPstatesInfoEx = (NvAPI_GPU_GetDynamicPstatesInfoEx_t)(*NvAPI_QueryInterface)(0x60DED2ED);

			NvAPI_Status res = NvAPI_Initialize();
			success = (res == NVAPI_OK);
		}
	}

	if (!success) {
		throw std::runtime_error("Unable to load NVAPI library!");
	}
}

void NvApiClient::getNvAPIError(NvAPI_Status status, std::string &message) const {
	NvAPI_ShortString nv_err;
	NvAPI_GetErrorMessage(status, nv_err);
	message = nv_err;
};

bool NvApiClient::I2CReadByteEx(NV_PHYSICAL_GPU_HANDLE gpu, byte deviceAddress, byte registerAddress, byte *data) const {
	NvAPI_Status status;
	NV_I2C_INFO pI2CInfo;

	pI2CInfo.version = NV_I2C_INFO_VER;
	pI2CInfo.displayMask = 0;
	pI2CInfo.bIsDDCPort = 0;
	pI2CInfo.i2cDevAddress = deviceAddress << 1;      //!< The address of the I2C slave.  The address should be shifted left by one.  For
												//!< example, the I2C address 0x50, often used for reading EDIDs, would be stored
												//!< here as 0xA0.  This matches the position within the byte sent by the master, as
												//!< the last bit is reserved to specify the read or write direction.
	pI2CInfo.pbI2cRegAddress = (NvU8*)(&registerAddress);    //!< The I2C target register address.  May be NULL, which indicates no register
												//!< address should be sent.
	pI2CInfo.regAddrSize = 1;        //!< The size in bytes of target register address.  If pbI2cRegAddress is NULL, this
											//!< field must be 0.
	pI2CInfo.pbData = (NvU8*)data;             //!< The buffer of data which is to be read or written (depending on the command).
	pI2CInfo.cbSize = 1;             //!< The size of the data buffer, pbData, to be read or written.
	pI2CInfo.i2cSpeed = NVAPI_I2C_SPEED_DEPRECATED;           //!< Deprecated, Must be set to NVAPI_I2C_SPEED_DEPRECATED.
	pI2CInfo.i2cSpeedKhz = NVAPI_I2C_SPEED_400KHZ;        //!< The target speed of the transaction in KHz (Chosen from the enum NV_I2C_SPEED).
	pI2CInfo.portId = 1;             //!< The portid on which device is connected (remember to set bIsPortIdSet if this value is set)
												//!< Optional for pre-Kepler
	pI2CInfo.bIsPortIdSet = 1;
	
	NvU32 unknown = 0;
	status = NvAPI_I2CReadEx(gpu, &pI2CInfo, &unknown);
	if (status != NVAPI_OK) {
		std::string message;
		getNvAPIError(status, message);
		std::cerr << "Error calling NvAPI_I2CReadEx: " << message << std::endl;
		return false;
	};
	return true;
}

bool NvApiClient::I2CWriteByteEx(NV_PHYSICAL_GPU_HANDLE gpu, byte deviceAddress, byte registerAddress, byte value) const {
	NvAPI_Status status;
	NV_I2C_INFO pI2CInfo;

	pI2CInfo.version = NV_I2C_INFO_VER;
	pI2CInfo.displayMask = 0;
	pI2CInfo.bIsDDCPort = 0;
	pI2CInfo.i2cDevAddress = deviceAddress << 1;      //!< The address of the I2C slave.  The address should be shifted left by one.  For
												//!< example, the I2C address 0x50, often used for reading EDIDs, would be stored
												//!< here as 0xA0.  This matches the position within the byte sent by the master, as
												//!< the last bit is reserved to specify the read or write direction.
	pI2CInfo.pbI2cRegAddress = (NvU8*)(&registerAddress);    //!< The I2C target register address.  May be NULL, which indicates no register
												//!< address should be sent.
	pI2CInfo.regAddrSize = 1;        //!< The size in bytes of target register address.  If pbI2cRegAddress is NULL, this
											//!< field must be 0.
	pI2CInfo.pbData = (NvU8*)(&value);             //!< The buffer of data which is to be read or written (depending on the command).
	pI2CInfo.cbSize = 1;             //!< The size of the data buffer, pbData, to be read or written.
	pI2CInfo.i2cSpeed = NVAPI_I2C_SPEED_DEPRECATED;           //!< Deprecated, Must be set to NVAPI_I2C_SPEED_DEPRECATED.
	pI2CInfo.i2cSpeedKhz = NVAPI_I2C_SPEED_400KHZ;        //!< The target speed of the transaction in KHz (Chosen from the enum NV_I2C_SPEED).
	pI2CInfo.portId = 1;             //!< The portid on which device is connected (remember to set bIsPortIdSet if this value is set)
												//!< Optional for pre-Kepler
	pI2CInfo.bIsPortIdSet = 1;

	NvU32 unknown = 0;
	status = NvAPI_I2CWriteEx(gpu, &pI2CInfo, &unknown);
	if (status != NVAPI_OK) {
		std::string message;
		getNvAPIError(status, message);
		std::cerr << "Error calling NvAPI_I2CWriteEx: " << message << std::endl;
		return false;
	};
	return true;
}

bool NvApiClient::getNvapiVersion(std::string &version) const {
	NvAPI_Status status;

	NvAPI_ShortString v;
	status = NvAPI_GetInterfaceVersionString(v);
	if (status != NVAPI_OK) {
		std::string message;
		getNvAPIError(status, message);
		std::cerr << "Error calling NvAPI_GetInterfaceVersionString: " << message << std::endl;
		return false;
	};

	version = v;
	return true;
};

bool NvApiClient::getGPUHandles(std::vector<NV_PHYSICAL_GPU_HANDLE> &list_gpus) const {
	NvAPI_Status status;

	NV_PHYSICAL_GPU_HANDLE gh[NVAPI_MAX_PHYSICAL_GPUS];
	NvS32 gpu_count = 0;
	status = NvAPI_EnumPhysicalGPUs(gh, &gpu_count);
	if (status != NVAPI_OK) {
		std::string message;
		getNvAPIError(status, message);
		std::cerr << "Error calling NvAPI_EnumPhysicalGPUs: " << message << std::endl;
		return false;
	};
	list_gpus.insert(list_gpus.end(), &gh[0], &gh[gpu_count]);
	return true;
}

bool NvApiClient::getGPUFullname(NV_PHYSICAL_GPU_HANDLE handle, std::string& name) const {
	NvAPI_Status status;

	NvAPI_ShortString n;
	status = NvAPI_GPU_GetFullName(handle, n);
	if ( status != NVAPI_OK) {
		std::string message;
		getNvAPIError(status, message);
		std::cerr << "Error calling getGPUFullname: " << message << std::endl;
		return false;
	};
	name = n;
	return true;
}

int NvApiClient::getExternalFanSpeedPercent(NV_PHYSICAL_GPU_HANDLE handle) const {
	bool res;
	
	NvU8 speed_hex;
	res = I2CReadByteEx(handle, I2C_EXTFAN_DEVICE_ADDRESS, I2C_EXTFAN_SPEED_CMD_REGISTER, &speed_hex);
	if (!res) {
		return -1;
	}
	return hexToPercent(speed_hex);
}

int NvApiClient::getExternalFanSpeedRPM(NV_PHYSICAL_GPU_HANDLE handle, int nb) const {
	bool res;
	int reg = 0;
	switch (nb) {
	case 1: 
		reg = I2C_EXTFAN1_SPEED_RPM_REGISTER; break;
	case 2:
		reg = I2C_EXTFAN2_SPEED_RPM_REGISTER; break;
	}

	NvU8 speed_rpm;
	res = I2CReadByteEx(handle, I2C_EXTFAN_DEVICE_ADDRESS, reg, &speed_rpm);
	if (!res) {
		return -1;
	}
	return speed_rpm * 30;
}

bool NvApiClient::setExternalFanSpeedPercent(NV_PHYSICAL_GPU_HANDLE handle, int percent) const {
	bool res;

	byte hex_val = percentToHex(percent);

	res = I2CWriteByteEx(handle, I2C_EXTFAN_DEVICE_ADDRESS, I2C_EXTFAN_SPEED_CMD_REGISTER, hex_val);
	if (!res) {
		return false;
	}
	return true;
}

bool NvApiClient::getTemps(NV_PHYSICAL_GPU_HANDLE handle, NV_GPU_THERMAL_SETTINGS& infos) const  {
	NvAPI_Status status;
	infos.version = NV_GPU_THERMAL_SETTINGS_VER_2;
	status = NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &infos);
	if (status != NVAPI_OK) {
		std::string message;
		getNvAPIError(status, message);
		std::cerr << "Error calling NvAPI_GPU_GetThermalSettings: " << message << std::endl;
		return false;
	};
	return true;
}

int NvApiClient::getGPUTemperature(NV_PHYSICAL_GPU_HANDLE handle) const {
	bool res = true;
	NV_GPU_THERMAL_SETTINGS infos{};
	infos.version = NV_GPU_THERMAL_SETTINGS_VER_2;
	res |= this->getTemps(handle, infos);
	if (!res) {
		return -1;
	}
	for (NvU32 i = 0; i < infos.count; i++) {
		NV_THERMAL_TARGET target = infos.sensor[i].target;
		switch (target) {
		case NVAPI_THERMAL_TARGET_GPU:
			return infos.sensor[i].currentTemp;
			break;
		}
	}
	return -1;
}

bool NvApiClient::detectI2CDevice(NV_PHYSICAL_GPU_HANDLE handle) const {
	bool res;
	// Asus GPUTweakII detection mechanism
	NvU8 high;
	NvU8 low;
	int chipId = 0;

	res = I2CReadByteEx(handle, I2C_EXTFAN_DEVICE_ADDRESS, I2C_DEVICE_IDENTIFIER_HIGH_REGISTER, &high);
	res &= I2CReadByteEx(handle, I2C_EXTFAN_DEVICE_ADDRESS, I2C_DEVICE_IDENTIFIER_LOW_REGISTER, &low);
	if (!res) {
		std::cerr << "Error reading bytes from I2C device during identification." << std::endl;
		return false;
	}

	chipId = (static_cast<int>(high) << 8) + static_cast<int>(low);
	if (chipId == I2C_IT8915_IDENTIFIER) {
		return true;
	}
	std::cerr << "Error detecting I2C device. Expecting device Id: " << std::hex << I2C_IT8915_IDENTIFIER << " but got " << chipId << " instead" << std::endl;
	return false;
}

int NvApiClient::getGPUUsage(NV_PHYSICAL_GPU_HANDLE& handle) {
	NvU32 utilization = -1;
	NV_GPU_DYNAMIC_PSTATES_INFO_EX GPUperf;
	GPUperf.version = MAKE_NVAPI_VERSION(NV_GPU_DYNAMIC_PSTATES_INFO_EX, 1);
	NvAPI_Status status = NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &GPUperf);
	if (status != NVAPI_OK) {
		std::string message;
		getNvAPIError(status, message);
		std::cerr << "Error calling NvAPI_GPU_GetThermalSettings: " << message << std::endl;
		return -1;
	}
	if (GPUperf.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].bIsPresent)
	{
		utilization = GPUperf.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].percentage;
	}
	return utilization;
};