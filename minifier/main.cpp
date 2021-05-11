#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdexcept>

#include "region_reader.hpp"
#include "zseg-writer.hpp"
#include "zstd-writer.hpp"

struct RegionMinifier {
	RegionReader rd;
	ZstdContext ctx;

	RegionMinifier(const char* filename, ZstdContext zctx)
		: rd(filename), ctx(zctx) {}
	RegionMinifier(const RegionMinifier& other) = delete;
	RegionMinifier(RegionMinifier&& other) = delete;

	void minify_segment(int segx, int segy, bool compressed=true, int dirfd=AT_FDCWD) {
		// TODO fixup segx / segy

		int fd = open_file(segx, segy, compressed, dirfd);
		if (fd < 0)
			throw std::runtime_error(strerror(errno));

		SegmentMinifier sm(rd, segx, segy);

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

private:
	int open_file(int segx, int segz, bool compressed, int dirfd) {
		char filename[64];
		sprintf(filename, "seg.%d.%d.%sseg", segx, segz, compressed? "z": "");

		return openat(dirfd, filename, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC, 0644);
	}
};

int main() {
	RegionMinifier rm("r.-1.0.mca", ZstdContext(15));

	rm.minify_segment(2, 3);
}
