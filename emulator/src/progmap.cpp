
#include <progmap.hpp>
#include <fstream>
#include <sstream>

static std::string getThing(std::fstream &fd) {
	
}

void SourceFile::init(std::string path) {
	std::fstream fd(path, std::fstream::in);
	
}

SourceFile::SourceFile() {}

SourceFile::SourceFile(std::string cwdPath, std::string path) {
	if (path[0] == '/') {
		init(path);
	} else {
		init(cwdPath + path);
	}
}

SourceFile::SourceFile(std::string path) {
	init(path);
}

SourceFile::SourceFile(const char *cwdPath, const char *path) {
	if (path[0] == '/') {
		init(path);
	} else {
		init(std::string(cwdPath) + std::string(path));
	}
}

SourceFile::SourceFile(const char *path) {
	init(path);
}
