#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <WinIoCtl.h>
#include <ntddscsi.h>
#include <conio.h>
#include <string>
#include <atlstr.h>

using namespace std;

#define BUFFER_SIZE 1024
#define PERSENTS 100
#define BYTE_SIZE 8

const char* bus_types[] = {
	"UNKNOWN",
	"SCSI",
	"ATAPI",
	"ATA",
	"ONE_TREE_NINE_FOUR",
	"SSA",
	"FIBRE",
	"USB",
	"RAID",
	"ISCSI",
	"SAS",
	"SATA",
	"SD",
	"MMC"
	"VIRTUAL",
	"FILE_BACKED_VIRTUAL"
};

void getDeviceInfo(HANDLE& drive_handle, STORAGE_PROPERTY_QUERY& storage_proterty_query) {
	//First call to get descriptor's size
	STORAGE_DESCRIPTOR_HEADER storage_descriptor_header = { 0 };
	DWORD bytes_returned = 0;
	if (!DeviceIoControl(drive_handle,
		IOCTL_STORAGE_QUERY_PROPERTY,
		&storage_proterty_query,
		sizeof(STORAGE_PROPERTY_QUERY),
		&storage_descriptor_header,
		sizeof(STORAGE_DESCRIPTOR_HEADER),
		&bytes_returned,
		NULL))
	{
		cout << GetLastError();
		CloseHandle(drive_handle);
		system("pause");
		exit(EXIT_FAILURE);
	}

	const DWORD out_buffer_size = storage_descriptor_header.Size;
	BYTE* out_buffer = new BYTE[out_buffer_size];
	ZeroMemory(out_buffer, out_buffer_size);

	//Second call to retreive data
	if (!DeviceIoControl(drive_handle,
		IOCTL_STORAGE_QUERY_PROPERTY,
		&storage_proterty_query,
		sizeof(STORAGE_PROPERTY_QUERY),
		out_buffer,
		out_buffer_size,
		&bytes_returned,
		NULL))
	{
		cout << GetLastError();
		CloseHandle(drive_handle);
		system("pause");
		exit(EXIT_FAILURE);
	}

	STORAGE_DEVICE_DESCRIPTOR* device_descriptor = (STORAGE_DEVICE_DESCRIPTOR*)out_buffer;
	device_descriptor->Size = out_buffer_size;

	if (device_descriptor->VendorIdOffset != 0)
	{
		cout << "Vendor: " << (char*)(out_buffer)+device_descriptor->VendorIdOffset << endl;
	}
	else
		cout << "Vendor: (Стандартные дисковые накопители)" << endl;
	if (device_descriptor->ProductIdOffset != 0)
	{
		cout << "Model: " << (char*)(out_buffer)+device_descriptor->ProductIdOffset << endl;
	}
	if (device_descriptor->ProductRevisionOffset != 0)
	{
		cout << "Firmware version: " << (char*)(out_buffer)+device_descriptor->ProductRevisionOffset << endl;
	}
	if (device_descriptor->BusType != 0)
	{
		cout << "Bus type: " << bus_types[device_descriptor->BusType] << endl;
	}
	if (device_descriptor->SerialNumberOffset != 0)
	{
		cout << "Serial number: " << (char*)(out_buffer)+device_descriptor->SerialNumberOffset << endl;
	}

	delete[] out_buffer;
}

void getMemoryInfo(DWORD drive_number, HANDLE& drive_handle, STORAGE_PROPERTY_QUERY& storage_proterty_query) {
	wstring path;
	_ULARGE_INTEGER diskSpace;
	_ULARGE_INTEGER freeSpace;

	diskSpace.QuadPart = 0;
	freeSpace.QuadPart = 0;

	_ULARGE_INTEGER totalDiskSpace;
	_ULARGE_INTEGER totalFreeSpace;

	totalDiskSpace.QuadPart = 0;
	totalFreeSpace.QuadPart = 0;

	//A bit mask, representing all the logical drives (volumes)
	unsigned long int logicalDrivesCount = GetLogicalDrives();

	for (char volume_letter = 'A'; volume_letter < 'Z'; volume_letter++) {
		if ((logicalDrivesCount >> volume_letter - 65) & 1 && volume_letter) {
			path = volume_letter;
			path.append(L":\\");

			GetDiskFreeSpaceEx(path.c_str(), 0, &diskSpace, &freeSpace);
			diskSpace.QuadPart = diskSpace.QuadPart / (BUFFER_SIZE * BUFFER_SIZE);
			freeSpace.QuadPart = freeSpace.QuadPart / (BUFFER_SIZE * BUFFER_SIZE);

			wstring open_path = L"\\\\.\\";
			open_path += volume_letter;
			open_path += L":";

			HANDLE volume_handle = CreateFile(
				open_path.c_str(),
				0,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			);
			if (volume_handle == INVALID_HANDLE_VALUE)
			{
				cout << GetLastError();
				CloseHandle(drive_handle);
				system("pause");
				exit(EXIT_FAILURE);
			}

			const DWORD out_buffer_size = 12;
			BYTE* out_buffer = new BYTE[out_buffer_size];
			ZeroMemory(out_buffer, out_buffer_size);
			DWORD bytes_returned = 0;

			if (!DeviceIoControl(volume_handle,
				IOCTL_STORAGE_GET_DEVICE_NUMBER,
				&storage_proterty_query,
				sizeof(STORAGE_PROPERTY_QUERY),
				out_buffer,
				out_buffer_size,
				&bytes_returned,
				NULL))
			{
				cout << GetLastError();
				CloseHandle(volume_handle);
				CloseHandle(drive_handle);
				system("pause");
				exit(EXIT_FAILURE);
			}

			_STORAGE_DEVICE_NUMBER* device_number = (_STORAGE_DEVICE_NUMBER*)out_buffer;

			if (device_number->DeviceNumber == drive_number) {
				if (diskSpace.QuadPart != 0)
				{
					totalDiskSpace.QuadPart += diskSpace.QuadPart;
					totalFreeSpace.QuadPart += freeSpace.QuadPart;

					cout << "-------------------" << endl;
					cout << "Volume: " << volume_letter << endl;
					cout << "Total space[Mb]" << diskSpace.QuadPart << endl;
					cout << "Free space[Mb]" << freeSpace.QuadPart << endl;
					cout << "Busy space[Mb]" << diskSpace.QuadPart - freeSpace.QuadPart << endl;
				}
			}

			delete[] out_buffer;
		}
	}
	cout << "-------------------";

	cout << endl;
	cout << "Overall Device's memory" << endl;
	cout << "Total space[Mb]" << totalDiskSpace.QuadPart << endl;
	cout << "Free space[Mb]" << totalFreeSpace.QuadPart << endl;
	cout << "Busy space[Mb]" << totalDiskSpace.QuadPart - totalFreeSpace.QuadPart << endl << endl;
}

VOID getVolumeCompliance(HANDLE drive_handle) {
	char VolumeNameBuffer[100];
	char FileSystemNameBuffer[100];
	unsigned long VolumeSerialNumber;

	BOOL GetVolumeInformationFlag = GetVolumeInformationA(
		"c:\\",
		VolumeNameBuffer,
		100,
		&VolumeSerialNumber,
		NULL, //&MaximumComponentLength,
		NULL, //&FileSystemFlags,
		FileSystemNameBuffer,
		100
	);

	if (GetVolumeInformationFlag)
	{
		cout << " Volume Name is " << VolumeNameBuffer << endl;
		cout << " Volume Serial Number is " << VolumeSerialNumber << endl;
		cout << " File System is " << FileSystemNameBuffer << endl;
	}
	else cout << " Not Present (GetVolumeInformation)" << endl;
}

HANDLE getDriveHandle(unsigned int drive_number) {
	CString strDrivePath;
	strDrivePath.Format(_T("\\\\.\\PhysicalDrive%u"), drive_number);

	// Get a handle to physical drive
	HANDLE drive_handle = CreateFile(
		strDrivePath,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (drive_handle == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	else
	{
		return drive_handle;
	}
}

int main()
{
	setlocale(LC_ALL, "rus");
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;	//Flag for retreiving divece's descriptor 
	storagePropertyQuery.QueryType = PropertyStandardQuery;		//Querry to return divece's descriptor

	HANDLE drive_handle;

	for (int i = 0; ; i++)
	{
		drive_handle = getDriveHandle(i);
		if (drive_handle == NULL)
		{
			break;
		}
		getDeviceInfo(drive_handle, storagePropertyQuery);
		getMemoryInfo(i, drive_handle, storagePropertyQuery);
		getVolumeCompliance(drive_handle);

		CloseHandle(drive_handle);
		cout << endl << endl;
	}

	system("pause");
	return 0;
}