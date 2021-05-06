#pragma once

#include <cstring>

#include <unordered_map>
#include <string>

// NOT!!!!! atomic
struct BlockIdPalette {
	std::unordered_map<std::string, int> p;
	int next = 0;

	int air;
	int removed;

	static const int nonsolid_border = 103;

	BlockIdPalette();

	bool is_solid(int id) {
		return id >= nonsolid_border;
	}

	int get_id(const char* key) {
		int& id = p[key];
		if (id == 0)
			id = next++;

		return id;
	}

	// todo: serialize to file
};

extern BlockIdPalette global_palette;
