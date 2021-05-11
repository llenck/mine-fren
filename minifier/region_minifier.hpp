#pragma once

#include <cstdio>
#include <cstring>
#include <cerrno>

#include <stdexcept>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "region_reader.hpp"
#include "zseg_writer.hpp"
#include "zstd_writer.hpp"

static int open_file(int segx, int segz, bool compressed, int dirfd) {
	char filename[64];
	sprintf(filename, "seg.%d.%d.%sseg", segx, segz, compressed? "z": "");

	int fd = openat(dirfd, filename, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC, 0644);

	if (fd < 0)
		throw std::runtime_error(strerror(errno));
	else
		return fd;
}

struct RegionMinifier {
	RegionReader rd;
	ZstdContext ctx;

	RegionMinifier(const char* filename, ZstdContext zctx)
		: rd(filename), ctx(zctx) {}
	RegionMinifier(const RegionMinifier& other) = delete;
	RegionMinifier(RegionMinifier&& other) = delete;

	void minify_segment(int segx, int segz, bool compressed=true, int dirfd=AT_FDCWD) {
		segx = ((segx % 4) + 4) % 4;
		segz = ((segz % 4) + 4) % 4;
		int abs_segx = segx + rd.x * 4;
		int abs_segz = segz + rd.z * 4;

		int fd = open_file(abs_segx, abs_segz, compressed, dirfd);

		SegmentMinifier sm(rd, segx, segz);

		if (compressed) {
			// create a zstd writer...
			ZstdWriter zwr(
				[=] (const uint8_t* p, size_t n) {
					return write(fd, p, n);
				},
				[=] () {
					close(fd);
					return true;
				},
			ctx);

			// ...which is fed from a zseg writer
			ZsegWriter wr(
				[&] (const uint8_t* p, size_t n, bool end) {
					return zwr.write(p, n, end);
				},
				[&] () {
					zwr.full_flush();
					return true;
				}
			);
			wr.put_minifier(sm);
		}
		else {
			// create a zseg writer directly, which just outputs to the file
			ZsegWriter wr(
				[&] (const uint8_t* p, size_t n, bool end) {
					ssize_t written = 0;
					do {
						ssize_t tmp = write(fd, p + written, n - written);
						if (tmp < 0)
							return tmp;

						written += tmp;
					} while (written < (ssize_t)n && end);

					return written;
				},
				[&] () {
					close(fd);
					return true;
				}
			);
			wr.put_minifier(sm);
		}
	}

	void minify_region(bool compressed=true, int dirfd=AT_FDCWD) {
		for (int i = 0; i < 16; i++) {
			minify_segment(i % 4, i / 4, compressed, dirfd);
		}
	}
};
