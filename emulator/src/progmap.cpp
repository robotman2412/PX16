
#include <progmap.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <window.h>

// Skips empty lines.
static bool readLine(std::ifstream &fd, std::string &out) {
	// Stream to store data in to.
	std::string buf;
	size_t cap = 0;
	
	// Iterate CharAcTerS.
	int c;
	while (1) {
		c = fd.get();
		
		if (c == EOF) {
			// When EOF, fin.
			if (buf.size() == 0) {
				return false;
			}
			break;
			
		} else if (c == '\n' || c == '\r') {
			// Consome LF after CR.
			if (c == '\r' && fd.peek() == '\n') fd.get();
			
			// When not empty, break.
			break;
			
		} else {
			// Add remainder to the buffer.
			buf += (char) c;
			cap ++;
		}
	}
	
	out = buf;
	return true;
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



Label::Label() {
	valid = false;
}

Label::Label(std::string line) {
	std::stringstream stream(line);
	valid = false;
	
	std::string tokens[3];
	for (int i = 0; i < 3; i++) {
		tokens[i] = readWord(stream);
		if (tokens[i].size() == 0) {
			std::cout << "Label incorrect: Missing token " << (i+1) << std::endl;
			return;
		}
	}
	
	if (tokens[0] != "label") {
		std::cout << "Label incorrect: Bad header" << std::endl;
		return;
	}
	
	name = tokens[1];
	
	try {
		address = std::stol(tokens[2], NULL, 16);
	} catch (std::invalid_argument) {
		std::cout << "Label incorrect: Bad number" << std::endl;
		return;
	}
	
	valid = true;
}



Section::Section() {
	valid = false;
}

Section::Section(std::string line) {
	std::stringstream stream(line);
	valid = false;
	
	std::string tokens[4];
	for (int i = 0; i < 4; i++) {
		tokens[i] = readWord(stream);
		if (tokens[i].size() == 0) {
			std::cout << "Section incorrect: Missing token " << (i+1) << std::endl;
			return;
		}
	}
	
	if (tokens[0] != "sect") {
		std::cout << "Section incorrect: Bad header" << std::endl;
		return;
	}
	
	name = tokens[1];
	
	try {
		address = std::stol(tokens[2], NULL, 16);
		size    = std::stol(tokens[3], NULL, 16);
	} catch (std::invalid_argument) {
		std::cout << "Section incorrect: Bad number" << std::endl;
		return;
	}
	
	valid = true;
}



SourceLine::SourceLine() {}

SourceLine::SourceLine(std::string line) {
	raw          = line;
	formatted    = mkMonoStr(style.text, line);
}



Pos::Pos() {
	valid = false;
}

Pos::Pos(std::string line) {
	std::stringstream stream(line);
	valid = false;
	
	std::string tokens[6];
	for (int i = 0; i < 6; i++) {
		tokens[i] = readWord(stream);
		if (tokens[i].size() == 0) {
			std::cout << "Pos incorrect: Missing token " << (i+1) << std::endl;
			return;
		}
	}
	
	if (tokens[0] != "pos") {
		std::cout << "Pos incorrect: Bad header" << std::endl;
		return;
	}
	
	absFile  = tokens[1];
	filename = tokens[2];
	try {
		address = std::stol(tokens[3], NULL, 16);
		size_t endptr = 0;
		x0      = std::stol(tokens[4], &endptr, 10);
		if (tokens[4][endptr] != ',') {
			std::cout << "Pos incorrect: Bad number separator" << std::endl;
			return;
		}
		y0      = std::stol(tokens[4].substr(endptr+1), NULL, 10);
		x1      = std::stol(tokens[5], &endptr, 10);
		if (tokens[5][endptr] != ',') {
			std::cout << "Pos incorrect: Bad number separator" << std::endl;
			return;
		}
		y1      = std::stol(tokens[5].substr(endptr+1), NULL, 10);
	} catch (std::invalid_argument) {
		std::cout << "Pos incorrect: Bad number" << std::endl;
		return;
	}
	
	valid = true;
}



void SourceFile::init(std::string path) {
	std::ifstream fd;
	fd.open(path);
	
	std::string line;
	while (readLine(fd, line)){
		lines.push_back(SourceLine(line));
		lines[lines.size()-1].linenumber = lines.size();
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



void ProgMap::init(std::string path) {
	std::ifstream fd;
	fd.open(path);
	
	// Read positions from the file.
	std::string line;
	while (readLine(fd, line)){
		if (!line.compare(0, 4, "pos ", 4)) {
			Pos pos(line);
			if (pos.valid) {
				posList.push_back(pos);
			}
			
		} else if (!line.compare(0, 5, "sect ", 5)) {
			Section section(line);
			if (section.valid) {
				sectionList.push_back(section);
			}
			
		} else if (!line.compare(0, 6, "label ", 6)) {
			Label label(line);
			if (label.valid) {
				labelList.push_back(label);
			}
		}
	}
	
	// Sort pos entries by address.
	std::sort(posList.begin(), posList.end(), [](Pos a, Pos b) {
		return a.address < b.address;
	});
	
	// Process list of pos entries.
	word pc = posList.size() ? posList[0].address : 0;
	for (size_t i = 0; i < posList.size(); i++) {
		Pos &pos = posList[i];
		
		// Map addresses to positions.
		while (pc <= pos.address) {
			posMap[pc] = pos;
			pc ++;
		}
		
		// Map absolute files to SRC FILE.
		if (fileMap.find(pos.absFile) == fileMap.end()) {
			fileMap[pos.absFile] = SourceFile(pos.absFile);
		}
	}
	
	// Map pos entries back to their files.
	for (size_t i = 0; i < posList.size(); i++) {
		Pos        &pos  = posList[i];
		SourceFile &file = fileMap[pos.absFile];
		SourceLine &line = file.lines[pos.y0-1];
		line.addresses.push_back(pos.address);
	}
	
	// Collect lines.
	for (auto iter = fileMap.begin(); iter != fileMap.end(); iter ++) {
		std::vector<SourceLine> &lines = iter->second.lines;
		lineList.insert(lineList.end(), lines.begin(), lines.end());
	}
	
	for (Section &sect: sectionList) {
		// Map names to sections.
		sectMap[sect.name] = sect;
		
		// Map sections to labels.
		word min = sect.address;
		word max = sect.address + sect.size;
		for (Label &label: labelList) {
			if (label.address >= min && label.address < max) {
				label.sect = sect.name;
			}
		}
	}
	
	// Map names to labels.
	for (Label &label: labelList) {
		labelMap[label.name] = label;
	}
}

ProgMap::ProgMap() {}

ProgMap::ProgMap(std::string cwdPath, std::string path) {
	if (path[0] == '/') {
		init(path);
	} else {
		init(cwdPath + path);
	}
}

ProgMap::ProgMap(std::string path) {
	init(path);
}

ProgMap::ProgMap(const char *cwdPath, const char *path) {
	if (path[0] == '/') {
		init(path);
	} else {
		init(std::string(cwdPath) + std::string(path));
	}
}

ProgMap::ProgMap(const char *path) {
	init(path);
}

Pos ProgMap::addr2line(word addr) {
	auto iter = posMap.find(addr);
	if (iter != posMap.end()) {
		return iter->second;
	} else {
		return Pos();
	}
}
