#pragma once

#undef _GNU_SOURCE
#define _GNU_SOURCE // for memfd_create
#include <sys/mman.h>
#include <unistd.h>
#include <cstdint>
#include <stdexcept>

template <typename T, bool atomic>
struct RingBuffer {
	uint8_t* base = nullptr;
	size_t sz = 0;
	off_t read_off = 0, write_off = 0;
	int fd = -1;

	RingBuffer(int pages=1) {
		if (sysconf(_SC_PAGESIZE) != 4096)
			return;

		sz = pages * 4096;

		// mmap all pages we'll use up front and later overwrite the mapping
		base = (uint8_t*)mmap(nullptr, sz * 2, PROT_NONE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (base == MAP_FAILED) {
			base = nullptr;
			return;
		}

		size_t elem_sz = sizeof(T);
		if (4096 / elem_sz * elem_sz != 4096)
			goto err_unmap;

		fd = memfd_create("mine-fren i/o ring buffer", MFD_CLOEXEC);
		if (fd < 0)
			goto err_unmap;

		if (ftruncate(fd, sz) < 0)
			goto err_close;

		if (mmap(base, sz, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_FIXED, fd, 0) == MAP_FAILED)
			goto err_close;

		if (mmap(base + sz, sz, PROT_READ | PROT_WRITE,
					MAP_SHARED | MAP_FIXED, fd, 0) == MAP_FAILED)
			goto err_close;

		return;

err_close:
		close(fd);
		fd = -1;
err_unmap:
		munmap(base, sz * 2);
		base = nullptr;

		throw std::runtime_error("Failed to create ring buffer");
	}

	RingBuffer(const RingBuffer& other) = delete;
	RingBuffer(RingBuffer&& other) = delete;

	~RingBuffer() {
		if (fd >= 0)
			close(fd);

		if (base != nullptr)
			munmap(base, sz * 2);
	}

#define ATOMIC_LOAD(x) (atomic? __atomic_load_n(&x, __ATOMIC_SEQ_CST) : x)
#define ATOMIC_XCHG(to, from, expected) \
	(atomic? \
		({to = from; 1;}) \
		: \
		__atomic_compare_exchange_n(&to, &expected, from, false, \
			__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) )

	const T* read_ptr() {
		return (const T*)(base + ATOMIC_LOAD(read_off));
	}
	size_t read_cap() {
		auto _read_off = ATOMIC_LOAD(read_off);
		auto _write_off = ATOMIC_LOAD(write_off);
		return ((_write_off - _read_off + sz) % sz) / sizeof(T);
	}
	void adv_read_ptr(off_t n) {
		off_t _read_off;
		off_t og_read_off;
		do {
			_read_off = ATOMIC_LOAD(read_off);
			og_read_off = _read_off;
			_read_off += n * sizeof(T);
			_read_off %= sz;
		} while (!ATOMIC_XCHG(read_off, _read_off, og_read_off));
	}

	T* write_ptr() {
		return (T*)(base + ATOMIC_LOAD(write_off));
	}
	size_t write_cap() {
		auto _write_off = ATOMIC_LOAD(write_off);
		auto _read_off = ATOMIC_LOAD(read_off);
		return ((_read_off - _write_off + sz - 1) % sz) / sizeof(T);
	}
	void adv_write_ptr(off_t n) {
		off_t _write_off;
		off_t og_write_off;
		do {
			_write_off = ATOMIC_LOAD(write_off);
			og_write_off = _write_off;
			_write_off += n * sizeof(T);
			_write_off %= sz;
		} while (!ATOMIC_XCHG(write_off, _write_off, og_write_off));
	}

#undef ATOMIC_LOAD
#undef ATOMIC_XCHG
};
