#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void printUsage()
{
	std::cout << "Usage: lnwin [source] [destination]" << std::endl;
}

int main(int argc, char** argv)
{
	if (argc != 4) {
		printUsage();
		return 1;
	}

	const char* switches = argv[1];
	bool createSymbolic = false;
	bool force = false;
	if (switches[0] != '-') {
		printUsage();
		return 1;
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
			return 0;
		default:
			printUsage();
			return 1;
		}
	}

	if (!createSymbolic) {
		std::cout << "Only symbolic links are supported at this time." << std::endl;
		return 1;
	}

	const char* target = argv[2];
	const char* linkName = argv[3];

	// Validate the target and linkname parameters
	if (target == nullptr || linkName == nullptr) {
		std::cout << "Invalid target or linkname" << std::endl;
		return 1;
	}

	// Check whether linkName already exists on disk
	if (GetFileAttributesA(linkName) != INVALID_FILE_ATTRIBUTES) {
		if (!force) {
			std::cout << "Link already exists. Use -f to force overwrite." << std::endl;
			return 1;
		}
	}

	// Determine whether the source is a file or a directory
	DWORD attributes = GetFileAttributesA(target);
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		std::cout << "Error: Unable to determine the type of the source file." << std::endl;
		return 1;
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
		
		std::cout << "Failed to create hard link to " << target << " at " << linkName << ": " << messageBuffer << std::endl;

		LocalFree(messageBuffer);
		return 1;
	}
	
	return 0;
}
