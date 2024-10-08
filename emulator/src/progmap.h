
#pragma once

#include <px16.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

class Label {
	public:
		Label();
		Label(std::string line);
		
		std::string sect;
		std::string	name;
		word		address;
		word        size;
		bool        valid;
};

class Section {
	public:
		Section();
		Section(std::string line);
		
		std::string	name;
		word		address;
		word        size;
		bool        valid;
};

class Pos {
	public:
		Pos();
		Pos(std::string line);
		
		std::string	absFile;
		std::string	filename;
		word		address;
		int			x0, y0;
		int			x1, y1;
		bool        valid;
};

class SourceLine {
	public:
		SourceLine();
		SourceLine(std::string line);
		
		int linenumber;
		std::vector<word> addresses;
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
	private:
		void init(std::string path);
		
	public:
		std::map<std::string, SourceFile> fileMap;
		std::map<word, Pos> posMap;
		std::vector<Pos> posList;
		std::vector<SourceLine> lineList;
		std::vector<Section> sectionList;
		std::vector<Label> labelList;
		std::map<std::string, Label> labelMap;
		std::map<std::string, Section> sectMap;
		
		ProgMap();
		ProgMap(std::string cwdPath, std::string path);
		ProgMap(std::string path);
		ProgMap(const char *cwdPath, const char *path);
		ProgMap(const char *path);
		
		Pos addr2line(word addr);
};
