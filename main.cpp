#include <cstdio>
#include <cstdint>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

struct RegionReader {
	int x, y;
	int off = 0;
	int fd;

	const uint8_t* map;
	size_t sz = 0;

	RegionReader(const char* filename) {
		if (sscanf(filename, "r.%d.%d.mca", &x, &y) != 2)
			return;

		if ((fd = open(filename, O_RDONLY | O_CLOEXEC)) < 0)
			return;

		struct stat file_info;
		if (fstat(fd, &file_info) < 0)
			goto err_clofd;

		sz = file_info.st_size;

		if ((map = (const uint8_t*)mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0))
				== MAP_FAILED)
			goto err_clofd;

		return; // success

err_clofd:
		close(fd);
		map = nullptr;
		sz = 0;
	}

	~RegionReader() {
		if (sz != 0) {
			munmap((void*)map, sz);
			sz = 0;
			close(fd);
		}
	}

	// mapping/fd -> "global-ish" state, destructors would interfere with copies
	RegionReader(const RegionReader& other) = delete;

	off_t chunk_off(int chunkx, int chunkz) {
		chunkx %= 32;
		chunkz %= 32;

		int chunk_info_off = chunkx + chunkz * 16;

		uint32_t* chunk_infos = (uint32_t*)map;
		uint32_t chunk_info = chunk_infos[chunk_info_off];

		// if both offset and sector count are zero, return -1
		if (chunk_info == 0)
			return -1;

		// otherwise, return the offset (first 3 bytes, big endian)
		uint32_t chunk_sector_offset = __builtin_bswap32(chunk_info) >> 8;

		return chunk_sector_offset * 4096;
	}

	bool err() {
		return sz == 0;
	}
};

int main() {
	RegionReader rd("r.0.0.mca");

	printf("Success: %d\n", !rd.err());

	for (int z = 0; z < 32; z++) {
		for (int x = 0; x < 32; x++) {
			printf("%d/%d -> %zd\n", x, z, rd.chunk_off(x, z));
		}
	}
}
