#include <cstdio>

#include <memory>

#include "region_reader.hpp"
#include "chunk_loader.hpp"
#include "palette.hpp"

#include "nbt.hpp"

// segment ^= 8x8 chunks
// segx = chunkx / 8
struct SegmentMinifier {
	Chunk* chunks[16 * 16] = {};

	// nx / px -> negative x, positive x
	Chunk* adj_nx[8] = {};
	Chunk* adj_nz[8] = {};
	Chunk* adj_px[8] = {};
	Chunk* adj_pz[8] = {};

	uint16_t* rows[16];
	uint16_t* row_ny = nullptr, * row_py = nullptr;
	int row_y = 0; // y of rows[0]

	uint16_t& block_ref(int x, int y, int z) {
		static uint16_t something_nonsolid = 0;

		int chunkx = x / 16;
		int chunkz = z / 16;
		Chunk* chunk = chunks[chunkx * 16 + chunkz];
		if (!chunk)
			return something_nonsolid;

		int chunk_local_x = x % 16;
		int chunk_local_z = z % 16;

		off_t n = Chunk::xyz_to_index(chunk_local_x, y, chunk_local_z);
		return chunk->blocks[n];
	}

	uint16_t block_const(int x, int y, int z) {
		static const uint16_t something_nonsolid = 0;

		// if out of bounds, assume air
		if (x < 0 || x >= 128
		 || y < 0 || y >= 256
		 || z < 0 || z >= 128)
			return something_nonsolid;

		// otherwise, do the usual dance
		return block_ref(x, y, z);
	}

	SegmentMinifier(const RegionReader& r, int segx, int segz) {
		segx %= 4;
		segz %= 4;

		int start_x = segx * 8;
		int start_z = segz * 8;

		bool any_present = false;

		for (int x = start_x; x < start_x + 8; x++) {
			for (int z = start_z; z < start_z + 8; z++) {
				Chunk*& cur = chunks[x * 16 + z];
				cur = new Chunk(r.get_chunk(x, z));

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
				adj_nx[z] = new Chunk(r.get_chunk(start_x - 1, z));
		}
		if (segx != 3) {
			for (int z = 0; z < 8; z++)
				adj_px[z] = new Chunk(r.get_chunk(start_x + 1, z));
		}
		if (segz != 0) {
			for (int x = 0; x < 8; x++)
				adj_nz[x] = new Chunk(r.get_chunk(x, start_z - 1));
		}
		if (segz != 3) {
			for (int x = 0; x < 8; x++)
				adj_pz[x] = new Chunk(r.get_chunk(x, start_z + 1));
		}
	}
	SegmentMinifier(const SegmentMinifier& other) = delete;
	SegmentMinifier(SegmentMinifier&& other) = delete;

	~SegmentMinifier() {
		for (int i = 0; i < 16 * 16; i++)
			delete chunks[i];

		Chunk** adjs[4] = { adj_nx, adj_nz, adj_px, adj_pz };
		for (int i = 0; i < 4; i++) {
			Chunk** adj = adjs[i];

			for (int j = 0; j < 8; j++)
				delete adj[j];
		}
	}

	void minify() {

	}
};

int main() {
	RegionReader rd("r.0.0.mca");

	if (rd.err())
		return 1;

	SegmentMinifier m(rd, 1, 1);
	m.minify();
}
