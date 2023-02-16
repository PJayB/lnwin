#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <filesystem>

void printUsage()
{
	std::cout << "Usage:" << std::endl;
	std::cout << "  lnwin -s [-f] <source> <destination>" << std::endl;
	std::cout << "  lnwin -?" << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 4) {
		printUsage();
		return EXIT_FAILURE;
	}

	const char* switches = argv[1];
	bool createSymbolic = false;
	bool force = false;
	if (switches[0] != '-') {
		printUsage();
		return EXIT_FAILURE;
	}
	for (int i = 1; switches[i] != '\0'; i++) {
		switch (switches[i]) {
		case 's':
			createSymbolic = true;
			break;
		case 'f':
			force = true;
			break;
		case 'h':
		case '?':
			printUsage();
			return EXIT_SUCCESS;
		default:
			printUsage();
			return EXIT_FAILURE;
		}
	}

	if (!createSymbolic) {
		std::cerr << "Only symbolic links are supported at this time." << std::endl;
		return 1;
	}

	const char* target = argv[2];
	const char* linkName = argv[3];

	// Validate the target and linkname parameters
	if (target == nullptr || linkName == nullptr) {
		std::cerr << "Invalid target or linkname" << std::endl;
		return 1;
	}

	// Check whether linkName already exists on disk
	auto attributes = GetFileAttributesA(linkName);
	if (attributes != INVALID_FILE_ATTRIBUTES) {
		if (!force) {
			std::cerr << linkName << " already exists. Use -f to force overwrite." << std::endl;
			return EXIT_FAILURE;
		}
		else if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0) {
			std::cerr << "Cowardly refusing to overwrite existing non-symlink: " << linkName << std::endl;
			return EXIT_FAILURE;
		}
		else if (!std::filesystem::remove(linkName)) {
			std::cerr << "Failed to remove existing link: " << linkName << " (" << GetLastError() << ")" << std::endl;
			return EXIT_FAILURE;
		}
	}

	// Determine whether the source is a file or a directory
	attributes = GetFileAttributesA(target);
	if (!force && attributes == INVALID_FILE_ATTRIBUTES) {
		std::cerr << "Error: Unable to determine the type of the source file." << std::endl;
		return EXIT_FAILURE;
	}

	// If it's a directory, add the directory flag
	DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
	if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
		flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
	}
	
	// Create a symbolic link between the source file and the destination file
	if (!CreateSymbolicLinkA(linkName, target, flags)) {
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		
		std::cerr << "Failed to create link to " << target << " at " << linkName << ": " << messageBuffer << std::endl;

		LocalFree(messageBuffer);
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
