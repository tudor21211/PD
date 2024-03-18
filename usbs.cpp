#include <initguid.h>
#include <usbiodef.h>
#include <windows.h>
#include <SetupAPI.h>
#include <devguid.h>
#include <RegStr.h>
#include <iostream>
#include <usbioctl.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

int main() {
	//enumerare device-uri usb
	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet == INVALID_HANDLE_VALUE) {
		std::cerr << "Error: SetupDiGetClassDevs failed." << std::endl;
		return 1;
	}

	//initializare interfata
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	//enumerare interfete
	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &GUID_DEVINTERFACE_USB_DEVICE, i, &deviceInterfaceData); ++i) {
		DWORD requiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);
		SP_DEVICE_INTERFACE_DETAIL_DATA* deviceInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(requiredSize);
		if (deviceInterfaceDetailData == NULL) {
			std::cerr << "Error: Failed to allocate memory for deviceInterfaceDetailData." << std::endl;
			continue;
		}
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, NULL, NULL)) {
			free(deviceInterfaceDetailData);
			continue;
		}


		// deschidere device
		HANDLE hDevice = CreateFile(deviceInterfaceDetailData->DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hDevice == INVALID_HANDLE_VALUE) {
			std::cerr << "Error: Failed to open device." << std::endl;
			free(deviceInterfaceDetailData);
			continue;
		}

		// luam datele despre device in handler-ul usbInfo
		USB_NODE_CONNECTION_INFORMATION usbInfo;
		DWORD bytesReturned;
		if (!DeviceIoControl(hDevice, IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, NULL, 0, &usbInfo, sizeof(usbInfo), &bytesReturned, NULL)) {
			std::cerr << "Error: Failed to query device information." << std::endl;
			CloseHandle(hDevice);
			free(deviceInterfaceDetailData);
			continue;
		}

		// extragem datele
		std::cout << "Device ID: " << usbInfo.DeviceDescriptor.idProduct << std::endl;

		CloseHandle(hDevice);
		free(deviceInterfaceDetailData);
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);

	return 0;
}