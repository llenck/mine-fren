#include "region_reader.hpp"

#include <cstring>

RegionReader::RegionReader(const char* filename) {
	const char* basename = strrchr(filename, '/');
	if (basename)
		basename += 1;
	else
		basename = filename;

	if (sscanf(basename, "r.%d.%d.mca", &x, &z) != 2)
		throw std::logic_error("Couldn't parse file name");

	if ((fd = open(filename, O_RDONLY | O_CLOEXEC)) < 0)
		throw std::runtime_error("Couldn't open region file");

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
	throw std::runtime_error("Couldn't map region file");
}
RegionReader::RegionReader(RegionReader&& other) {
	x = other.x;
	z = other.z;
	off = other.off;
	map = std::exchange(other.map, nullptr);
	sz = std::exchange(other.sz, 0);
}

RegionReader::~RegionReader() {
	if (sz != 0) {
		munmap((void*)map, sz);
		sz = 0;
		close(fd);
	}
}

off_t RegionReader::chunk_off(int chunkx, int chunkz) const {
	chunkx %= 32;
	chunkz %= 32;

	int chunk_info_off = chunkx + chunkz * 32;

	uint32_t* chunk_infos = (uint32_t*)map;
	uint32_t chunk_info = chunk_infos[chunk_info_off];

	// if both offset and sector count are zero, return -1
	if (chunk_info == 0)
		throw std::runtime_error("Chunk doesn't exist");

	// otherwise, return the offset (first 3 bytes, big endian)
	uint32_t chunk_sector_offset = __builtin_bswap32(chunk_info) >> 8;

	return chunk_sector_offset * 4096;
}

RawChunkView RegionReader::get_chunk(int chunkx, int chunkz) const {
	off_t off = chunk_off(chunkx, chunkz);
	if (off < 8192)
		throw std::runtime_error("Chunk data inside table");

	const uint8_t* chunk = map + off;
	if (chunk[5] >= 128)
		return RawChunkView(chunkx, chunkz);

	uint32_t len = *((uint32_t*)chunk);
	len = __builtin_bswap32(len);

	if (len + (off + 4) > (off_t)sz)
		throw std::runtime_error("Chunk data beyond EOF");
	else
		return RawChunkView(chunk + 5, len);
}
