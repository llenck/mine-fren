#pragma once

#include <cstring>

#include <unordered_map>
#include <string>

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

	std::string serialize() {
		std::string ret("{");

		bool first = true;

		for (auto kv : p) {
			char next[64];
			sprintf(next, "%s\n\t\"%s\": %d", first? "" : ",", kv.first.c_str(), kv.second);
			first = false;

			ret += next;
		}
		ret += "\n}";

		return ret;
	}
};
