#include <iostream>
#include <windows.h>
#include <setupapi.h>

using namespace std;

#pragma comment (lib, "Setupapi.lib")

int main() {
  
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	HDEVINFO device_info_set;
	SP_DEVINFO_DATA device_info_data;      
	
	device_info_set = SetupDiGetClassDevs(    
		NULL,                                   
		"PCI",
		NULL,
		DIGCF_PRESENT |							
		DIGCF_ALLCLASSES						
	);

	if (device_info_set == INVALID_HANDLE_VALUE) {
		exit(GetLastError());
	}

	device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);  
	for (DWORD i = 0; SetupDiEnumDeviceInfo(device_info_set, i, &device_info_data); i++) {  
																							

		DWORD  required_size_hardware_id;

		SetupDiGetDeviceRegistryProperty(device_info_set, &device_info_data, SPDRP_HARDWAREID, 
			NULL, NULL, 0, &required_size_hardware_id);    
																			
		PBYTE property_buffer_hardware_id = new BYTE[required_size_hardware_id];
		BOOL status_device = SetupDiGetDeviceRegistryProperty(device_info_set, &device_info_data, SPDRP_HARDWAREID,
			NULL, property_buffer_hardware_id, required_size_hardware_id, nullptr);

		if (!status_device) {
			SetupDiDestroyDeviceInfoList(device_info_set);  
			exit(GetLastError());
		}

		string hardware_id(reinterpret_cast<char*>(property_buffer_hardware_id));

		SetupDiGetDeviceRegistryProperty(device_info_set, &device_info_data, SPDRP_DEVICEDESC,
			NULL, NULL, 0, &required_size_hardware_id);

		PBYTE property_buffer_description = new BYTE[required_size_hardware_id];
		BOOL status_desctiption = SetupDiGetDeviceRegistryProperty(device_info_set, &device_info_data, SPDRP_DEVICEDESC,
			NULL, property_buffer_description, required_size_hardware_id, nullptr);
    
		if (!status_desctiption) {
			SetupDiDestroyDeviceInfoList(device_info_set);
			exit(GetLastError());
		}

		string device_description(reinterpret_cast<char*>(property_buffer_description));

		cout << device_description << endl;

		size_t venPos = hardware_id.find("VEN_");
		size_t devPos = hardware_id.find("DEV_");

		cout << "VendorID : " << hardware_id.substr(venPos + 4, 4) << endl;
		cout << "DeviceID : " << hardware_id.substr(devPos + 4, 4) << endl;

		cout << endl;
	}

	SetupDiDestroyDeviceInfoList(device_info_set);
	return 0;
}
