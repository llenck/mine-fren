#pragma once

#include <array>
#include <memory>

#include "region_reader.hpp"
#include "chunk_loader.hpp"

using chunk_data_t = std::array<uint16_t, 64 * 256 * 256>;

static constexpr int seg_xyz_to_index(int x, int y, int z) {
	return x + 128 * (z + 128 * y);
}

// segment ^= 8x8 chunks
// segx = chunkx / 8
struct SegmentMinifier {
	Chunk* chunks[8 * 8] = {};

	// nx / px -> negative x, positive x
	Chunk* adj_nx[8] = {};
	Chunk* adj_nz[8] = {};
	Chunk* adj_px[8] = {};
	Chunk* adj_pz[8] = {};

	BlockIdPalette palette;

	SegmentMinifier(const RegionReader& r, int segx, int segz);
	SegmentMinifier(const SegmentMinifier& other) = delete;
	SegmentMinifier(SegmentMinifier&& other) = delete;

	~SegmentMinifier();

	std::unique_ptr<chunk_data_t> minify();

private:
	// may only be used for blocks within the segment
	uint16_t get_block(int x, int y, int z);
	// returns 0 for blocks outside of segment
	uint16_t get_block_const(int x, int y, int z);

	bool has_nonsolid_neighbours(int x, int y, int z);
	void minify_chunk(int chunk_x, int chunk_z, uint16_t* data);
};
