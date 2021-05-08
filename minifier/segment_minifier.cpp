#include "segment_minifier.hpp"

#include "palette.hpp"

SegmentMinifier::SegmentMinifier(const RegionReader& r, int segx, int segz) {
	segx %= 4;
	segz %= 4;

	int start_x = segx * 8;
	int start_z = segz * 8;

	bool any_present = false;

	for (int x = start_x; x < start_x + 8; x++) {
		for (int z = start_z; z < start_z + 8; z++) {
			int a_x = x % 8;
			int a_z = z % 8;

			Chunk*& cur = chunks[a_x * 8 + a_z];
			cur = new Chunk(palette, r.get_chunk(x, z));

			if (cur->err()) {
				delete cur;
				cur = nullptr;
			}
			else
				any_present = true;
		}
	}

	// return early if this segment is empty
	if (!any_present)
		return;

	if (segx != 0) {
		for (int z = 0; z < 8; z++)
			adj_nx[z] = new Chunk(palette, r.get_chunk(start_x - 1, z));
	}
	if (segx != 3) {
		for (int z = 0; z < 8; z++)
			adj_px[z] = new Chunk(palette, r.get_chunk(start_x + 1, z));
	}
	if (segz != 0) {
		for (int x = 0; x < 8; x++)
			adj_nz[x] = new Chunk(palette, r.get_chunk(x, start_z - 1));
	}
	if (segz != 3) {
		for (int x = 0; x < 8; x++)
			adj_pz[x] = new Chunk(palette, r.get_chunk(x, start_z + 1));
	}
}

SegmentMinifier::~SegmentMinifier() {
	for (int i = 0; i < 8 * 8; i++)
		delete chunks[i];

	Chunk** adjs[4] = { adj_nx, adj_nz, adj_px, adj_pz };
	for (int i = 0; i < 4; i++) {
		Chunk** adj = adjs[i];

		for (int j = 0; j < 8; j++)
			delete adj[j];
	}
}

std::unique_ptr<chunk_data_t> SegmentMinifier::minify() {
	std::unique_ptr<chunk_data_t> ret{new chunk_data_t()};

	for (int x = 0; x < 8; x++) {
		for (int z = 0; z < 8; z++) {
			minify_chunk(x, z, ret->data());
		}
	}

	return ret;
}

uint16_t SegmentMinifier::get_block(int x, int y, int z) {
	int chunkx = x / 16;
	int chunkz = z / 16;
	Chunk* chunk = chunks[chunkx * 8 + chunkz];
	if (!chunk)
		return palette.air;

	int chunk_local_x = x % 16;
	int chunk_local_z = z % 16;

	off_t n = Chunk::xyz_to_index(chunk_local_x, y, chunk_local_z);
	return chunk->blocks[n];
}

// returns 0 for blocks outside of segment
uint16_t SegmentMinifier::get_block_const(int x, int y, int z) {
	static const uint16_t something_nonsolid = 0;

	// if out of bounds, assume air
	if (x < 0 || x >= 128
	 || y < 0 || y >= 256
	 || z < 0 || z >= 128)
		return something_nonsolid;

	// otherwise, do the usual dance
	return get_block(x, y, z);
}

bool SegmentMinifier::has_nonsolid_neighbours(int x, int y, int z) {
	return
		!palette.is_solid(get_block_const(x, y - 1, z)) ||
		!palette.is_solid(get_block_const(x, y + 1, z)) ||
		!palette.is_solid(get_block_const(x - 1, y, z)) ||
		!palette.is_solid(get_block_const(x + 1, y, z)) ||
		!palette.is_solid(get_block_const(x, y, z - 1)) ||
		!palette.is_solid(get_block_const(x, y, z + 1));
}

void SegmentMinifier::minify_chunk(int chunk_x, int chunk_z, uint16_t* data) {
	int start_x = chunk_x * 16;
	int start_z = chunk_z * 16;

	if (!chunks[chunk_x * 8 + chunk_z]) {
		// chunk doesn't exist; fill with air
		for (int y = 0; y < 256; y++)
			for (int z = start_z; z < start_z + 16; z++)
				for (int x = start_x; x < start_x + 16; x++)
					data[seg_xyz_to_index(x, y, z)] = palette.air;

		return;
	}

	// a small speedup could be achieved by caching whether an adjacent chunk
	// exists. if it doesn't, we could use a special loop that copies all blocks
	// that are not air without checking neighbours
	for (int y = 0; y < 256; y++) {
		for (int z = start_z; z < start_z + 16; z++) {
			for (int x = start_x; x < start_x + 16; x++) {
				int nth = seg_xyz_to_index(x, y, z);
				uint16_t block = get_block(x, y, z);

				if (!palette.is_solid(block)) {
					data[nth] = block;
					continue;
				}

				if (has_nonsolid_neighbours(x, y, z))
					data[nth] = block;
				else
					data[nth] = palette.removed;
			}
		}
	}
}
