#pragma comment(lib, "powrprof.lib")
#include <Windows.h>
#include <iostream>
#include <powrprof.h>

using namespace std;

int main() {

	setlocale(LC_ALL, "Rus");
	SYSTEM_POWER_STATUS batteryStatus;
	SetSuspendState(false, false, false);

	while (true) {
		if (GetSystemPowerStatus(&batteryStatus) == 0) {
			cout << GetLastError() << endl;
			system("pause");
			return -1;
		}

		switch (batteryStatus.ACLineStatus) {
		case 0:
			cout << "Состояние питания аккумулятора: отключено" << endl;
			break;
		case 1:
			cout << "Состояние питания аккумулятора: подключено" << endl;
			break;
		default:
			cout << "Состояние питания аккумулятора: неизвестно" << endl;
			break;
		}

		switch (batteryStatus.BatteryFlag) {
		case 0:
			cout << "Состояние заряда аккумулятора: батарея не заряжается, а емкость батареи находится между низким и высоким" << endl;
			break;
		case 1:
			cout << "Состояние заряда аккумулятора: high ( > 66% )" << endl;
			break;
		case 2:
			cout << "Состояние заряда аккумулятора: low ( > 33% )" << endl;
			break;
		case 4:
			cout << "Состояние заряда аккумулятора: critical ( < 5% )" << endl;
			break;
		case 8:
			cout << "Состояние заряда аккумулятора: charging" << endl;
			break;
		case 128:
			cout << "Состояние заряда аккумулятора: no system battery" << endl;
			break;
		default:
			cout << "Состояние заряда аккумулятора: unknown " << endl;
			break;
		}

		cout << "Текущий процент заряда: " << (int)batteryStatus.BatteryLifePercent << " %" << endl;

		switch (batteryStatus.SystemStatusFlag) {
		case 0:
			cout << "Режим энергосбережения отключён" << endl;
			break;
		case 1:
			cout << "Режим энергосбережения включён" << endl;
			break;
		}

		Sleep(1000);
		system("cls");
	}
	return 0;
}