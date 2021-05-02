#pragma once

#include <cstring>

#include <map>
#include <string>

// NOT!!!!! atomic
struct BlockIdPalette {
	std::map<std::string, int> p;
	int next = 0;

	int air;
	int cave_air;

	static const int nonsolid_border = 104;

	BlockIdPalette();

	bool is_air(int id) {
		return id == air || id == cave_air;
	}

	bool is_solid(int id) {
		return id >= nonsolid_border;
	}

	int get_id(const char* key) {
		// 0 is the default value for ints, so we must handle air, which maps to 0,
		// as a special case
		if (!strcmp(key, "air"))
			return 0;

		int& id = p[key];
		if (id == 0)
			id = next++;

		return id;
	}

	// todo: serialize to file
};

extern BlockIdPalette global_palette;
