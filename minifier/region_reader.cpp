#include "region_reader.hpp"

RegionReader::RegionReader(const char* filename) {
	if (sscanf(filename, "r.%d.%d.mca", &x, &z) != 2)
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

RawChunkView RegionReader::get_chunk(int chunkx, int chunkz) const {
	off_t off = chunk_off(chunkx, chunkz);
	if (off < 8192)
		return {};

	const uint8_t* chunk = map + off;
	if (chunk[5] >= 128)
		return RawChunkView(chunkx, chunkz);

	uint32_t len = *((uint32_t*)chunk);
	len = __builtin_bswap32(len);

	if (len + (off + 4) > (off_t)sz)
		return {}; // error if chunk goes beyond eof
	else
		return RawChunkView(chunk + 5, len);
}

bool RegionReader::err() const {
	return sz == 0;
}
