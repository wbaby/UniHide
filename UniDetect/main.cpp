#include "drives.h"
#include "common.h"

int main() {
	DriveDetector driveDetector;

	for (auto descriptor : driveDetector.descriptors) {
		printf("Serial for device %C: %s\n", descriptor.driver_letter, descriptor.serial.c_str());
	}

	system("pause");

	return 0;
}