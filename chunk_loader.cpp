#include <cstdio>
#include <cstring>

#include <algorithm>
#include <functional>

#include "chunk_loader.hpp"
#include "palette.hpp"

#include "nbt.hpp"

struct PointerWithOff {
	const uint8_t* data;
	const size_t sz;
	size_t off;
};

static size_t nbt_read_cb(void* userdata, uint8_t* data, size_t size) {
	PointerWithOff* p = (PointerWithOff*)userdata;

	size_t left = p->sz - p->off;
	if (left <= 0)
		return 0;

	size_t to_read = left < size? left : size;

	memcpy(data, p->data + p->off, to_read);

	p->off += to_read;

	return to_read;
}

static nbt_tag* parse_nbt(const RawChunkView& v) {
	if (v.err())
		return nullptr;

	PointerWithOff state = { v.data, v.sz, 0 };
	nbt_reader_t reader = { nbt_read_cb, &state };
	return nbt_parse(reader, NBT_PARSE_FLAG_USE_ZLIB);
}

static nbt_tag* get_tag(nbt_tag* root, const char* key) {
	if (!root || root->type != NBT_TYPE_COMPOUND)
		return nullptr;
	else
		return nbt_tag_compound_get(root, key);
}

Chunk::Chunk(const RawChunkView& v) {
	nbt_tag* _root = parse_nbt(v);
	if (!_root)
		return;

	std::unique_ptr<nbt_tag, std::function<void(nbt_tag*)>> root(_root,
		[] (nbt_tag* t) { nbt_free_tag(t); });

	blocks.reset(new uint16_t[16 * 16 * 256]());
	std::fill_n(blocks.get(), 16 * 16 * 256, (uint16_t)global_palette.air);

	// get sections
	nbt_tag* secs = get_tag(get_tag(root.get(), "Level"), "Sections");
	if (!secs
		|| secs->type != NBT_TYPE_LIST
		|| secs->tag_list.type != NBT_TYPE_COMPOUND)
		goto err;

	// for each section, get the palette and block tags, and pass them to parse_section
	for (unsigned i = 0; i < secs->tag_list.size; i++) {
		nbt_tag* sec = secs->tag_list.value[i];

		nbt_tag* palette = get_tag(sec, "Palette");
		if (!palette
			|| palette->type != NBT_TYPE_LIST
			|| palette->tag_list.type != NBT_TYPE_COMPOUND)
		{
			continue;
		}

		nbt_tag* blocks = get_tag(sec, "BlockStates");
		if (!blocks || blocks->type != NBT_TYPE_LONG_ARRAY) {
			continue;
		}

		nbt_tag* y_tag = get_tag(sec, "Y");
		if (!y_tag || y_tag->type != NBT_TYPE_BYTE) {
			continue;
		}

		int y = y_tag->tag_byte.value;

		parse_section(palette, blocks, y);
	}

	return;
err:
	blocks.reset(nullptr);
}

bool Chunk::err() const {
	return !blocks;
}

void Chunk::parse_section(nbt_tag* palette, nbt_tag* blocks, int sector_y) {
	long long num_palette_entries = palette->tag_list.size;
	std::unique_ptr<uint16_t[]> section_palette(new uint16_t[num_palette_entries]);
	std::fill_n(section_palette.get(), num_palette_entries,
			(uint16_t)global_palette.air);

#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))
#define UPPER_LOG2(X) (LOG2(X - 1) + 1)

	int bits_per_id;
	if (num_palette_entries >= 16)
		bits_per_id = std::max((int)UPPER_LOG2(num_palette_entries), 4);
	else
		bits_per_id = 4;

#undef LOG2
#undef UPPER_LOG2

	int blocks_per_long = 64 / bits_per_id;

	unsigned longs_required = (16 * 16 * 16 - 1) / blocks_per_long + 1;

	if (blocks->tag_long_array.size != longs_required) {
		fprintf(stderr, "%d: wrong number of longs (%zd != %d)\n", __LINE__,
				blocks->tag_long_array.size, longs_required);
		return;
	}

	for (unsigned i = 0; i < num_palette_entries; i++) {
		nbt_tag* e = palette->tag_list.value[i];

		nbt_tag* id = get_tag(e, "Name");
		if (!id || id->type != NBT_TYPE_STRING) {
			fprintf(stderr, "%d: error finding id\n", __LINE__);
			return;
		}

		const char* block_name = strchr(id->tag_string.value, ':');
		if (!block_name) {
			fprintf(stderr, "%d: no properly namespaced block id %s\n", __LINE__,
					id->tag_string.value);
			return;
		}

		block_name++;
		section_palette[i] = global_palette.get_id(block_name);
	}

	int n = 0;
	int mask = (1 << bits_per_id) - 1;
	for (unsigned i = 0; i < longs_required; i++) {
		uint64_t cur = blocks->tag_long_array.value[i];

		for (int j = 0; j < blocks_per_long; j++) {
			int nth_block = n++;
			if (nth_block >= 16 * 16 * 16)
				break;

			int local_id = cur & mask;
			cur >>= bits_per_id;

			int glob_id = section_palette[local_id];

//			int x = nth_block % 16;
//			int z = (nth_block / 16) % 16;
//			int y = (nth_block / (16 * 16)) % 16 + sector_y * 16;
//			printf("%d / %d / %d -> %d\n", x, y, z, glob_id);

			off_t idx = nth_to_index(nth_block, sector_y);
			this->blocks[idx] = glob_id;
		}
	}
}
