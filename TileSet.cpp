#include "TileSet.hpp"

#include <array>
#include <fstream>
#include <stdexcept>
#include <sstream>

TileSet::TileSet(std::string filename) {
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw new std::runtime_error("Failed to load file for TileSet");
	}

	std::string line, num;

	int linenum = 0;
	while (file.good()) {
		std::getline(file, line);
		std::stringstream linestream(line);

		int col = 0;
		if (linenum < 4) {
			//Parsing palette
			std::array< uint8_t, 4 > rgba = {0, 0, 0, 0};
			while (std::getline(linestream, num, ',')) {
				if (col >= 4) {
					throw new std::runtime_error("Malformed palette");
				}
				rgba[col] = (uint8_t)stoi(num);
				col++;
			}
			palette[linenum] = glm::u8vec4(rgba[0], rgba[1], rgba[2], rgba[3]);
		} else {
			//Parsing tile
			std::array< uint8_t, 8 > bit0 = {0, 0, 0, 0, 0, 0, 0, 0};
			std::array< uint8_t, 8 > bit1 = {0, 0, 0, 0, 0, 0, 0, 0};
			while (std::getline(linestream, num, ',')) {
				if (col < 8) {
					bit0[col] = (uint8_t)stoi(num);
				} else if (col < 16) {
					bit1[col - 8] = (uint8_t)stoi(num);
				} else {
					throw new std::runtime_error("Malformed tile");
				}
				col++;
			}

			tiles.emplace_back();
			tiles.back().bit0 = bit0;
			tiles.back().bit1 = bit1;
		}

		linenum++;
	}
}