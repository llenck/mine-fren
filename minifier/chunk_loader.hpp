#pragma once

#include <cstdint>

#include <memory>

#include "region_reader.hpp"
#include "palette.hpp"

#include "nbt.hpp"

struct Chunk {
	std::unique_ptr<uint16_t[]> blocks{};
	BlockIdPalette& shared_palette;

	Chunk(BlockIdPalette& p, const RawChunkView& _v);
	Chunk(const Chunk& other) = delete;
	Chunk(Chunk&& other) = delete; // can be implemented if necessary

	static constexpr off_t xyz_to_index(int x, int y, int z) {
		return x + (z + y * 16) * 16;
	}
	static constexpr off_t nth_to_index(int nth_block, int sector_y) {
		return nth_block + sector_y * (16 * 16 * 16);
	}

	private:
	void parse_section(nbt_tag* palette, nbt_tag* blocks, int y);
};
