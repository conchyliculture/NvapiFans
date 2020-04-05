#pragma once
#include <windows.h>
#include <cstdint>
#include <iostream>
#include <vector>

typedef std::uint8_t byte;

// From https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/group__nvapitypes.html
#define 	NVAPI_GENERIC_STRING_MAX   4096
#define 	NVAPI_LONG_STRING_MAX   256
#define 	NVAPI_SHORT_STRING_MAX   64

#define NVAPI_MAX_PHYSICAL_GPUS 64

typedef float NvF32;
typedef signed short NvS16;
typedef signed int NvS32;
typedef __int64 NvS64;
typedef signed char NvS8;
typedef unsigned short NvU16;
typedef unsigned long NvU32;
typedef unsigned __int64 NvU64;
typedef unsigned char NvU8;
typedef unsigned long temp_NvU32;
typedef NvU8 NvBool;
typedef char    NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];
typedef char 	NvAPI_String[NVAPI_GENERIC_STRING_MAX];
typedef char 	NvAPI_LongString[NVAPI_LONG_STRING_MAX];
typedef char 	NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];
typedef NvU16 	NvAPI_UnicodeShortString[NVAPI_SHORT_STRING_MAX];

typedef NvS32* NV_HANDLE;
typedef NV_HANDLE NV_PHYSICAL_GPU_HANDLE;

// from NVIDIA's nvapi.h
typedef enum _NvAPI_Status
{
    NVAPI_OK = 0,      // Success. Request is completed.
    NVAPI_ERROR = -1,      // Generic error
    NVAPI_LIBRARY_NOT_FOUND = -2,      // NVAPI support library cannot be loaded.
    NVAPI_NO_IMPLEMENTATION = -3,      // not implemented in current driver installation
    NVAPI_API_NOT_INITIALIZED = -4,      // NvAPI_Initialize has not been called (successfully)
    NVAPI_INVALID_ARGUMENT = -5,      // The argument/parameter value is not valid or NULL.
    NVAPI_NVIDIA_DEVICE_NOT_FOUND = -6,      // No NVIDIA display driver, or NVIDIA GPU driving a display, was found.
    NVAPI_END_ENUMERATION = -7,      // No more items to enumerate
    NVAPI_INVALID_HANDLE = -8,      // Invalid handle
    NVAPI_INCOMPATIBLE_STRUCT_VERSION = -9,      // An argument's structure version is not supported
    NVAPI_HANDLE_INVALIDATED = -10,     // The handle is no longer valid (likely due to GPU or display re-configuration)
    NVAPI_OPENGL_CONTEXT_NOT_CURRENT = -11,     // No NVIDIA OpenGL context is current (but needs to be)
    NVAPI_INVALID_POINTER = -14,     // An invalid pointer, usually NULL, was passed as a parameter
    NVAPI_NO_GL_EXPERT = -12,     // OpenGL Expert is not supported by the current drivers
    NVAPI_INSTRUMENTATION_DISABLED = -13,     // OpenGL Expert is supported, but driver instrumentation is currently disabled
    NVAPI_NO_GL_NSIGHT = -15,     // OpenGL does not support Nsight

    NVAPI_EXPECTED_LOGICAL_GPU_HANDLE = -100,    // Expected a logical GPU handle for one or more parameters
    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE = -101,    // Expected a physical GPU handle for one or more parameters
    NVAPI_EXPECTED_DISPLAY_HANDLE = -102,    // Expected an NV display handle for one or more parameters
    NVAPI_INVALID_COMBINATION = -103,    // The combination of parameters is not valid.
    NVAPI_NOT_SUPPORTED = -104,    // Requested feature is not supported in the selected GPU
    NVAPI_PORTID_NOT_FOUND = -105,    // No port ID was found for the I2C transaction
    NVAPI_EXPECTED_UNATTACHED_DISPLAY_HANDLE = -106,    // Expected an unattached display handle as one of the input parameters.
    NVAPI_INVALID_PERF_LEVEL = -107,    // Invalid perf level
    NVAPI_DEVICE_BUSY = -108,    // Device is busy; request not fulfilled
    NVAPI_NV_PERSIST_FILE_NOT_FOUND = -109,    // NV persist file is not found
    NVAPI_PERSIST_DATA_NOT_FOUND = -110,    // NV persist data is not found
    NVAPI_EXPECTED_TV_DISPLAY = -111,    // Expected a TV output display
    NVAPI_EXPECTED_TV_DISPLAY_ON_DCONNECTOR = -112,    // Expected a TV output on the D Connector - HDTV_EIAJ4120.
    NVAPI_NO_ACTIVE_SLI_TOPOLOGY = -113,    // SLI is not active on this device.
    NVAPI_SLI_RENDERING_MODE_NOTALLOWED = -114,    // Setup of SLI rendering mode is not possible right now.
    NVAPI_EXPECTED_DIGITAL_FLAT_PANEL = -115,    // Expected a digital flat panel.
    NVAPI_ARGUMENT_EXCEED_MAX_SIZE = -116,    // Argument exceeds the expected size.
    NVAPI_DEVICE_SWITCHING_NOT_ALLOWED = -117,    // Inhibit is ON due to one of the flags in NV_GPU_DISPLAY_CHANGE_INHIBIT or SLI active.
    NVAPI_TESTING_CLOCKS_NOT_SUPPORTED = -118,    // Testing of clocks is not supported.
    NVAPI_UNKNOWN_UNDERSCAN_CONFIG = -119,    // The specified underscan config is from an unknown source (e.g. INF)
    NVAPI_TIMEOUT_RECONFIGURING_GPU_TOPO = -120,    // Timeout while reconfiguring GPUs
    NVAPI_DATA_NOT_FOUND = -121,    // Requested data was not found
    NVAPI_EXPECTED_ANALOG_DISPLAY = -122,    // Expected an analog display
    NVAPI_NO_VIDLINK = -123,    // No SLI video bridge is present
    NVAPI_REQUIRES_REBOOT = -124,    // NVAPI requires a reboot for the settings to take effect
    NVAPI_INVALID_HYBRID_MODE = -125,    // The function is not supported with the current Hybrid mode.
    NVAPI_MIXED_TARGET_TYPES = -126,    // The target types are not all the same
    NVAPI_SYSWOW64_NOT_SUPPORTED = -127,    // The function is not supported from 32-bit on a 64-bit system.
    NVAPI_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED = -128,    // There is no implicit GPU topology active. Use NVAPI_SetHybridMode to change topology.
    NVAPI_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS = -129,      // Prompt the user to close all non-migratable applications.
    NVAPI_OUT_OF_MEMORY = -130,    // Could not allocate sufficient memory to complete the call.
    NVAPI_WAS_STILL_DRAWING = -131,    // The previous operation that is transferring information to or from this surface is incomplete.
    NVAPI_FILE_NOT_FOUND = -132,    // The file was not found.
    NVAPI_TOO_MANY_UNIQUE_STATE_OBJECTS = -133,    // There are too many unique instances of a particular type of state object.
    NVAPI_INVALID_CALL = -134,    // The method call is invalid. For example, a method's parameter may not be a valid pointer.
    NVAPI_D3D10_1_LIBRARY_NOT_FOUND = -135,    // d3d10_1.dll cannot be loaded.
    NVAPI_FUNCTION_NOT_FOUND = -136,    // Couldn't find the function in the loaded DLL.
    NVAPI_INVALID_USER_PRIVILEGE = -137,    // Current User is not Admin.
    NVAPI_EXPECTED_NON_PRIMARY_DISPLAY_HANDLE = -138,    // The handle corresponds to GDIPrimary.
    NVAPI_EXPECTED_COMPUTE_GPU_HANDLE = -139,    // Setting Physx GPU requires that the GPU is compute-capable.
    NVAPI_STEREO_NOT_INITIALIZED = -140,    // The Stereo part of NVAPI failed to initialize completely. Check if the stereo driver is installed.
    NVAPI_STEREO_REGISTRY_ACCESS_FAILED = -141,    // Access to stereo-related registry keys or values has failed.
    NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED = -142, // The given registry profile type is not supported.
    NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED = -143,    // The given registry value is not supported.
    NVAPI_STEREO_NOT_ENABLED = -144,    // Stereo is not enabled and the function needed it to execute completely.
    NVAPI_STEREO_NOT_TURNED_ON = -145,    // Stereo is not turned on and the function needed it to execute completely.
    NVAPI_STEREO_INVALID_DEVICE_INTERFACE = -146,    // Invalid device interface.
    NVAPI_STEREO_PARAMETER_OUT_OF_RANGE = -147,    // Separation percentage or JPEG image capture quality is out of [0-100] range.
    NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED = -148, // The given frustum adjust mode is not supported.
    NVAPI_TOPO_NOT_POSSIBLE = -149,    // The mosaic topology is not possible given the current state of the hardware.
    NVAPI_MODE_CHANGE_FAILED = -150,    // An attempt to do a display resolution mode change has failed.
    NVAPI_D3D11_LIBRARY_NOT_FOUND = -151,    // d3d11.dll/d3d11_beta.dll cannot be loaded.
    NVAPI_INVALID_ADDRESS = -152,    // Address is outside of valid range.
    NVAPI_STRING_TOO_SMALL = -153,    // The pre-allocated string is too small to hold the result.
    NVAPI_MATCHING_DEVICE_NOT_FOUND = -154,    // The input does not match any of the available devices.
    NVAPI_DRIVER_RUNNING = -155,    // Driver is running.
    NVAPI_DRIVER_NOTRUNNING = -156,    // Driver is not running.
    NVAPI_ERROR_DRIVER_RELOAD_REQUIRED = -157,    // A driver reload is required to apply these settings.
    NVAPI_SET_NOT_ALLOWED = -158,    // Intended setting is not allowed.
    NVAPI_ADVANCED_DISPLAY_TOPOLOGY_REQUIRED = -159,    // Information can't be returned due to "advanced display topology".
    NVAPI_SETTING_NOT_FOUND = -160,    // Setting is not found.
    NVAPI_SETTING_SIZE_TOO_LARGE = -161,    // Setting size is too large.
    NVAPI_TOO_MANY_SETTINGS_IN_PROFILE = -162,    // There are too many settings for a profile.
    NVAPI_PROFILE_NOT_FOUND = -163,    // Profile is not found.
    NVAPI_PROFILE_NAME_IN_USE = -164,    // Profile name is duplicated.
    NVAPI_PROFILE_NAME_EMPTY = -165,    // Profile name is empty.
    NVAPI_EXECUTABLE_NOT_FOUND = -166,    // Application not found in the Profile.
    NVAPI_EXECUTABLE_ALREADY_IN_USE = -167,    // Application already exists in the other profile.
    NVAPI_DATATYPE_MISMATCH = -168,    // Data Type mismatch
    NVAPI_PROFILE_REMOVED = -169,    // The profile passed as parameter has been removed and is no longer valid.
    NVAPI_UNREGISTERED_RESOURCE = -170,    // An unregistered resource was passed as a parameter.
    NVAPI_ID_OUT_OF_RANGE = -171,    // The DisplayId corresponds to a display which is not within the normal outputId range.
    NVAPI_DISPLAYCONFIG_VALIDATION_FAILED = -172,    // Display topology is not valid so the driver cannot do a mode set on this configuration.
    NVAPI_DPMST_CHANGED = -173,    // Display Port Multi-Stream topology has been changed.
    NVAPI_INSUFFICIENT_BUFFER = -174,    // Input buffer is insufficient to hold the contents.
    NVAPI_ACCESS_DENIED = -175,    // No access to the caller.
    NVAPI_MOSAIC_NOT_ACTIVE = -176,    // The requested action cannot be performed without Mosaic being enabled.
    NVAPI_SHARE_RESOURCE_RELOCATED = -177,    // The surface is relocated away from video memory.
    NVAPI_REQUEST_USER_TO_DISABLE_DWM = -178,    // The user should disable DWM before calling NvAPI.
    NVAPI_D3D_DEVICE_LOST = -179,    // D3D device status is D3DERR_DEVICELOST or D3DERR_DEVICENOTRESET - the user has to reset the device.
    NVAPI_INVALID_CONFIGURATION = -180,    // The requested action cannot be performed in the current state.
    NVAPI_STEREO_HANDSHAKE_NOT_DONE = -181,    // Call failed as stereo handshake not completed.
    NVAPI_EXECUTABLE_PATH_IS_AMBIGUOUS = -182,    // The path provided was too short to determine the correct NVDRS_APPLICATION
    NVAPI_DEFAULT_STEREO_PROFILE_IS_NOT_DEFINED = -183,    // Default stereo profile is not currently defined
    NVAPI_DEFAULT_STEREO_PROFILE_DOES_NOT_EXIST = -184,    // Default stereo profile does not exist
    NVAPI_CLUSTER_ALREADY_EXISTS = -185,    // A cluster is already defined with the given configuration.
    NVAPI_DPMST_DISPLAY_ID_EXPECTED = -186,    // The input display id is not that of a multi stream enabled connector or a display device in a multi stream topology
    NVAPI_INVALID_DISPLAY_ID = -187,    // The input display id is not valid or the monitor associated to it does not support the current operation
    NVAPI_STREAM_IS_OUT_OF_SYNC = -188,    // While playing secure audio stream, stream goes out of sync
    NVAPI_INCOMPATIBLE_AUDIO_DRIVER = -189,    // Older audio driver version than required
    NVAPI_VALUE_ALREADY_SET = -190,    // Value already set, setting again not allowed.
    NVAPI_TIMEOUT = -191,    // Requested operation timed out
    NVAPI_GPU_WORKSTATION_FEATURE_INCOMPLETE = -192,    // The requested workstation feature set has incomplete driver internal allocation resources
    NVAPI_STEREO_INIT_ACTIVATION_NOT_DONE = -193,    // Call failed because InitActivation was not called.
    NVAPI_SYNC_NOT_ACTIVE = -194,    // The requested action cannot be performed without Sync being enabled.
    NVAPI_SYNC_MASTER_NOT_FOUND = -195,    // The requested action cannot be performed without Sync Master being enabled.
    NVAPI_INVALID_SYNC_TOPOLOGY = -196,    // Invalid displays passed in the NV_GSYNC_DISPLAY pointer.
    NVAPI_ECID_SIGN_ALGO_UNSUPPORTED = -197,    // The specified signing algorithm is not supported. Either an incorrect value was entered or the current installed driver/hardware does not support the input value.
    NVAPI_ECID_KEY_VERIFICATION_FAILED = -198,    // The encrypted public key verification has failed.
    NVAPI_FIRMWARE_OUT_OF_DATE = -199,    // The device's firmware is out of date.
    NVAPI_FIRMWARE_REVISION_NOT_SUPPORTED = -200,    // The device's firmware is not supported.
} NvAPI_Status;

//! Used in NV_GPU_THERMAL_SETTINGS
typedef enum
{
    NVAPI_THERMAL_TARGET_NONE = 0,
    NVAPI_THERMAL_TARGET_GPU = 1,     //!< GPU core temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_MEMORY = 2,     //!< GPU memory temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_POWER_SUPPLY = 4,     //!< GPU power supply temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_BOARD = 8,     //!< GPU board ambient temperature requires NvPhysicalGpuHandle
    NVAPI_THERMAL_TARGET_VCD_BOARD = 9,     //!< Visual Computing Device Board temperature requires NvVisualComputingDeviceHandle
    NVAPI_THERMAL_TARGET_VCD_INLET = 10,    //!< Visual Computing Device Inlet temperature requires NvVisualComputingDeviceHandle
    NVAPI_THERMAL_TARGET_VCD_OUTLET = 11,    //!< Visual Computing Device Outlet temperature requires NvVisualComputingDeviceHandle

    NVAPI_THERMAL_TARGET_ALL = 15,
    NVAPI_THERMAL_TARGET_UNKNOWN = -1,
} NV_THERMAL_TARGET;

//! \ingroup gputhermal
//! Used in NV_GPU_THERMAL_SETTINGS
typedef enum
{
    NVAPI_THERMAL_CONTROLLER_NONE = 0,
    NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL,
    NVAPI_THERMAL_CONTROLLER_ADM1032,
    NVAPI_THERMAL_CONTROLLER_MAX6649,
    NVAPI_THERMAL_CONTROLLER_MAX1617,
    NVAPI_THERMAL_CONTROLLER_LM99,
    NVAPI_THERMAL_CONTROLLER_LM89,
    NVAPI_THERMAL_CONTROLLER_LM64,
    NVAPI_THERMAL_CONTROLLER_ADT7473,
    NVAPI_THERMAL_CONTROLLER_SBMAX6649,
    NVAPI_THERMAL_CONTROLLER_VBIOSEVT,
    NVAPI_THERMAL_CONTROLLER_OS,
    NVAPI_THERMAL_CONTROLLER_UNKNOWN = -1,
} NV_THERMAL_CONTROLLER;

#define NVAPI_MAX_THERMAL_SENSORS_PER_GPU   3

typedef struct
{
    NvU32   version;                //!< structure version
    NvU32   count;                  //!< number of associated thermal sensors
    struct
    {
        NV_THERMAL_CONTROLLER       controller;         //!< internal, ADM1032, MAX6649...
        NvS32                       defaultMinTemp;     //!< Minimum default temperature value of the thermal sensor in degree Celsius
        NvS32                       defaultMaxTemp;     //!< Maximum default temperature value of the thermal sensor in degree Celsius
        NvS32                       currentTemp;        //!< Current temperature value of the thermal sensor in degree Celsius
        NV_THERMAL_TARGET           target;             //!< Thermal sensor targeted - GPU, memory, chipset, powersupply, Visual Computing Device, etc
    } sensor[NVAPI_MAX_THERMAL_SENSORS_PER_GPU];

} NV_GPU_THERMAL_SETTINGS_V2;

#define NVAPI_MAX_SIZEOF_I2C_DATA_BUFFER    4096
#define NVAPI_MAX_SIZEOF_I2C_REG_ADDRESS       4
#define NVAPI_DISPLAY_DEVICE_MASK_MAX         24
#define NVAPI_I2C_SPEED_DEPRECATED        0xFFFF

typedef enum
{
    NVAPI_I2C_SPEED_DEFAULT,    //!< Set i2cSpeedKhz to I2C_SPEED_DEFAULT if default I2C speed is to be chosen, ie.use the current frequency setting.
    NVAPI_I2C_SPEED_3KHZ,
    NVAPI_I2C_SPEED_10KHZ,
    NVAPI_I2C_SPEED_33KHZ,
    NVAPI_I2C_SPEED_100KHZ,
    NVAPI_I2C_SPEED_200KHZ,
    NVAPI_I2C_SPEED_400KHZ,
} NV_I2C_SPEED;

#define NV_GPU_THERMAL_SETTINGS_VER_2   MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SETTINGS_V2, 2)
typedef NV_GPU_THERMAL_SETTINGS_V2  NV_GPU_THERMAL_SETTINGS;

//! Used in NvAPI_I2CRead() and NvAPI_I2CWrite()
typedef struct
{
    NvU32                   version;            //!< The structure version.
    NvU32                   displayMask;        //!< The Display Mask of the concerned display.
    NvU8                    bIsDDCPort;         //!< This flag indicates either the DDC port (TRUE) or the communication port
                                                //!< (FALSE) of the concerned display.
    NvU8                    i2cDevAddress;      //!< The address of the I2C slave.  The address should be shifted left by one.  For
                                                //!< example, the I2C address 0x50, often used for reading EDIDs, would be stored
                                                //!< here as 0xA0.  This matches the position within the byte sent by the master, as
                                                //!< the last bit is reserved to specify the read or write direction.
    NvU8* pbI2cRegAddress;    //!< The I2C target register address.  May be NULL, which indicates no register
                                                //!< address should be sent.
    NvU32                   regAddrSize;        //!< The size in bytes of target register address.  If pbI2cRegAddress is NULL, this
                                                //!< field must be 0.
    NvU8* pbData;             //!< The buffer of data which is to be read or written (depending on the command).
    NvU32                   cbSize;             //!< The size of the data buffer, pbData, to be read or written.
    NvU32                   i2cSpeed;           //!< Deprecated, Must be set to NVAPI_I2C_SPEED_DEPRECATED.
    NV_I2C_SPEED            i2cSpeedKhz;        //!< The target speed of the transaction in KHz (Chosen from the enum NV_I2C_SPEED).
    NvU8                    portId;             //!< The portid on which device is connected (remember to set bIsPortIdSet if this value is set)
                                                //!< Optional for pre-Kepler
    NvU32                   bIsPortIdSet;       //!< set this flag on if and only if portid value is set
} NV_I2C_INFO_V3;

typedef NV_I2C_INFO_V3 	NV_I2C_INFO;

#define MAKE_NVAPI_VERSION(typeName, ver) (NvU32)(sizeof(typeName) | ((ver) << 16))
#define NV_I2C_INFO_VER3   MAKE_NVAPI_VERSION(NV_I2C_INFO_V3, 3)
#define NV_I2C_INFO_VER   NV_I2C_INFO_VER3


// function pointer types
typedef int* (*NvAPI_QueryInterface_t)(unsigned int offset);
typedef NvAPI_Status(*NvAPI_Initialize_t)();
typedef NvAPI_Status(*NvAPI_EnumPhysicalGPUs_t)(NV_PHYSICAL_GPU_HANDLE* physical_gpu_handles, NvS32* gpu_count);
typedef NvAPI_Status(*NvAPI_GetInterfaceVersionString_t)(NvAPI_ShortString str);
typedef NvAPI_Status(*NvAPI_GPU_GetFullName_t)(NV_PHYSICAL_GPU_HANDLE physical_gpu_handle, NvAPI_ShortString szName);
typedef NvAPI_Status(*NvAPI_GetErrorMessage_t)(NvAPI_Status nr, NvAPI_ShortString szDesc);
typedef NvAPI_Status(*NvAPI_I2CReadEx_t)(NV_PHYSICAL_GPU_HANDLE hPhysicalGpu, NV_I2C_INFO* pI2cInfo, NvU32* unknown); // No idea what unknown is
typedef NvAPI_Status(*NvAPI_I2CWriteEx_t)(NV_PHYSICAL_GPU_HANDLE hPhysicalGpu, NV_I2C_INFO* pI2cInfo, NvU32* unknown); // No idea what unknown is
typedef NvAPI_Status(*NvAPI_GPU_GetThermalSettings_t)(NV_PHYSICAL_GPU_HANDLE hPhysicalGpu, NvU32 sensorIndex, NV_GPU_THERMAL_SETTINGS* pThermalSettings);


#define I2C_EXTFAN_DEVICE_ADDRESS 0x2a
#define I2C_EXTFAN_SPEED_CMD_REGISTER 0x41
#define I2C_EXTFAN1_SPEED_RPM_REGISTER 0x44
#define I2C_EXTFAN2_SPEED_RPM_REGISTER 0x48
#define I2C_GPU_VRM_TEMP_REGISTER 0x15


class NvApiClient
{
private:
    HMODULE hNvapi;

	// nvapi.dll internal function pointers
	NvAPI_QueryInterface_t      NvAPI_QueryInterface = nullptr;
	NvAPI_Initialize_t          NvAPI_Initialize = nullptr;
	NvAPI_EnumPhysicalGPUs_t    NvAPI_EnumPhysicalGPUs = nullptr;
	NvAPI_GetInterfaceVersionString_t       NvAPI_GetInterfaceVersionString = nullptr;
	NvAPI_GPU_GetFullName_t		NvAPI_GPU_GetFullName = nullptr;
    NvAPI_GetErrorMessage_t     NvAPI_GetErrorMessage = nullptr;
    NvAPI_I2CReadEx_t      NvAPI_I2CReadEx = nullptr;
    NvAPI_I2CWriteEx_t  NvAPI_I2CWriteEx = nullptr;
    NvAPI_GPU_GetThermalSettings_t NvAPI_GPU_GetThermalSettings = nullptr;

    void getNvAPIError(NvAPI_Status status, std::string message);
    bool I2CReadByteEx(NV_PHYSICAL_GPU_HANDLE &gpu, byte deviceAddress, byte registerAddress, byte *data);
    bool I2CWriteByteEx(NV_PHYSICAL_GPU_HANDLE& gpu, byte deviceAddress, byte registerAddress, byte value);
    int hexToPercent(byte hex);
    int percentToHex(byte hex);

public:
	NvApiClient();
	bool showVersion(std::string& version);
	bool getGPUHandles(std::vector<NV_PHYSICAL_GPU_HANDLE>& gpuHandles);
	bool getGPUFullname(NV_PHYSICAL_GPU_HANDLE& handle, std::string& name);
    int getExternalFanSpeedPercent(NV_PHYSICAL_GPU_HANDLE& handle);
    int getExternalFanSpeedRPM(NV_PHYSICAL_GPU_HANDLE& handle, int nb);
    bool setExternalFanSpeedPercent(NV_PHYSICAL_GPU_HANDLE& handle, int percent);
    bool getTemps(NV_PHYSICAL_GPU_HANDLE& handle, NV_GPU_THERMAL_SETTINGS& infos);

};

