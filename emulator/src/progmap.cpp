
#include <progmap.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

// Skips empty lines.
static std::string readLine(std::ifstream &fd) {
	// Stream to store data in to.
	std::string buf;
	size_t cap = 0;
	
	// Iterate CharAcTerS.
	int c;
	while (1) {
		c = fd.get();
		
		if (c == EOF) {
			// When EOF, fin.
			break;
			
		} else if (c == '\n' || c == '\r') {
			// Consome LF after CR.
			if (c == '\r' && fd.peek() == '\n') fd.get();
			
			// When not empty, break.
			if (cap) break;
			
		} else {
			// Add remainder to the buffer.
			buf += (char) c;
			cap ++;
		}
	}
	
	return buf;
}

static std::string readWord(std::stringstream &fd) {
	// Stream to store data in to.
	std::string buf;
	
	// Iterate CharAcTerS.
	int c;
	while (1) {
		c = fd.get();
		
		if (c == EOF) {
			// When EOF, fin.
			break;
			
		} else if (c == '\\') {
			// Literally interpres the CHARACTER.
			buf += (char) fd.get();
			
		} else if (c == ' ' || c == '\t') {
			// Consume remaining whitespace.
			while (fd.peek() == ' ' || fd.peek() == '\t') fd.get();
			break;
			
		} else {
			// Add remainder to the buffer.
			buf += (char) c;
		}
	}
	
	// Get string from buffer.
	return buf;
}

void SourceFile::init(std::string path) {
	std::ifstream fd;
	fd.open(path);
	
	std::string line;
	while ((line = readLine(fd)).size()){
		std::cout << "Read line: " << line << std::endl;
	}
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
