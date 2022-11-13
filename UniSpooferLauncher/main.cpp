#include <Windows.h>
#include <iostream>

#include "hdd.h"

int main() {
	hdd::Error e = hdd::SetupHDD();

	if (e == hdd::Error::AccessDenied) {
		std::cout << "Access denied, please start me as administrator\n";
	}
	else if (e == hdd::Error::NotFound) {
		std::cout << "Could not finish setup due to unfound RVA\n";
	}
	else {
		std::cout << "Successfully finished setup for HDD\n";
	}

	system("pause");

	return 0;
}