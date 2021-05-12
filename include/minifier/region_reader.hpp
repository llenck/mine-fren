#pragma once

#include <cstdio>
#include <cstdint>

#include <utility>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

struct RawChunkView {
	const uint8_t* data = nullptr;
	uint32_t sz = 0;
	int fd = -1;

	RawChunkView(const uint8_t* _data, long unsigned _sz) : data(_data), sz(_sz) {}
	RawChunkView(int chunkx, int chunkz) {
		char filename[64];
		sprintf(filename, "c.%d.%d.mcc", chunkx, chunkz);

		if ((fd = open(filename, O_RDONLY | O_CLOEXEC)) < 0)
			throw std::runtime_error("Couldn't open chunk file");

		struct stat file_info;
		if (fstat(fd, &file_info) < 0)
			goto err;

		sz = file_info.st_size;

		if ((data = (const uint8_t*)mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0))
				== MAP_FAILED)
			goto err;

		return; // success
err:
		close(fd);
		fd = -1;
		data = nullptr;
		sz = 0;
		throw std::runtime_error("Couldn't map chunk file");
	}
	RawChunkView(const RawChunkView& other) = delete;
	RawChunkView(RawChunkView&& other) {
		data = std::exchange(other.data, nullptr);
		sz = std::exchange(other.sz, 0);
		fd = std::exchange(other.sz, -1);
	}

	~RawChunkView() {
		if (fd >= 0) {
			munmap((void*)data, sz);
			sz = 0;
			close(fd);
		}
	}
};

struct RegionReader {
	int x, z;
	int off = 0;
	int fd;

	const uint8_t* map;
	size_t sz = 0;

	RegionReader(const char* filename);
	// mapping/fd -> "global-ish" state, destructors would interfere with copies
	RegionReader(const RegionReader& other) = delete;
	RegionReader(RegionReader&& other);

	~RegionReader();

	off_t chunk_off(int chunkx, int chunkz) const;

	RawChunkView get_chunk(int chunkx, int chunkz) const;
};
