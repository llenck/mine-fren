#pragma once

#include <cstdint>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct FileView {
	const uint8_t* map = nullptr;
	uint32_t sz = 0;
	int fd = -1;

	FileView(const char* f, int dirfd=AT_FDCWD) {
		if ((fd = openat(dirfd, f, O_RDONLY | O_CLOEXEC)) < 0)
			return;

		struct stat file_info;
		if (fstat(fd, &file_info) < 0) {
			close(fd);
			return;
		}

		sz = file_info.st_size;

		if ((map = (const uint8_t*)mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0))
				== MAP_FAILED)
		{
			close(fd);
			map = nullptr;
		}

		return; // success
	}
	~FileView() {
		if (fd >= 0)
			close(fd);
		if (map != nullptr)
			munmap((void*)map, sz);
	}

	FileView(const FileView& other) = delete;
	FileView(FileView&& other) = delete;

	bool err() const {
		return map == nullptr;
	}
};
