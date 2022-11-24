
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
	private:
		void init(std::string path);
		
	public:
		SourceFile();
		SourceFile(std::string cwdPath, std::string path);
		SourceFile(std::string path);
		SourceFile(const char *cwdPath, const char *path);
		SourceFile(const char *path);
		
		std::string realPath;
		std::vector<SourceLine> lines;
};

class ProgMap {
	public:
		std::map<std::string, SourceFile> files;
		std::map<word, Pos> posMap;
		
		ProgMap();
		ProgMap(FILE *fd);
};
