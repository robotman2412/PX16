
#pragma once

#include <px16.h>
#include <string>
#include <vector>
#include <map>

class Pos {
	std::string	filename;
	word		address;
	int			x0, y0;
	int			x1, y1;
};

class SourceLine {
	public:
		std::string raw;
		std::string formatted;
};

class SourceFile {
	public:
		FILE *fd;
		std::vector<SourceLine> lines;
};

class ProgMap {
	public:
		std::map<std::string, SourceFile> files;
		std::map<word, Pos> posMap;
		
		ProgMap();
		ProgMap(FILE *fd);
};
