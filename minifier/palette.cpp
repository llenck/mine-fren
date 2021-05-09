#include "palette.hpp"

BlockIdPalette::BlockIdPalette() {
	static constexpr std::array<const char*, nonsolid_border> nonsolid_blocks{
		"stone_pressure_plate",
		"oak_pressure_plate",
		"spruce_pressure_plate",
		"birch_pressure_plate",
		"dark_oak_pressure_plate",
		"jungle_pressure_plate",
		"acacia_pressure_plate",
		"crimson_pressure_plate",
		"warped_pressure_plate",
		"oak_fence",
		"spruce_fence",
		"birch_fence",
		"dark_oak_fence",
		"jungle_fence",
		"acacia_fence",
		"crimson_fence",
		"warped_fence",
		"stone_slab",
		"oak_slab",
		"spruce_slab",
		"birch_slab",
		"dark_oak_slab",
		"jungle_slab",
		"acacia_slab",
		"crimson_slab",
		"warped_slab",
		"stone_stairs",
		"oak_stairs",
		"spruce_stairs",
		"birch_stairs",
		"dark_oak_stairs",
		"jungle_stairs",
		"acacia_stairs",
		"crimson_stairs",
		"warped_stairs",

		"cobblestone_wall",
		"granite_wall",
		"polished_granite_wall",
		"andesite_wall",
		"polished_andesite_wall",
		"diorite_wall",
		"polished_diorite_wall",
		"sandstone_wall",
		"smooth_sandstone_wall",
		"red_nether_brick_wall",
		"nether_brick_wall",
		"prismarine_wall",
		"prismarine_brick_wall",
		"dark_prismarine_wall",
		"cobblestone_stairs",
		"granite_stairs",
		"polished_granite_stairs",
		"andesite_stairs",
		"polished_andesite_stairs",
		"diorite_stairs",
		"polished_diorite_stairs",
		"sandstone_stairs",
		"smooth_sandstone_stairs",
		"red_nether_brick_stairs",
		"nether_brick_stairs",
		"prismarine_stairs",
		"prismarine_brick_stairs",
		"dark_prismarine_stairs",
		"cobblestone_slab",
		"granite_slab",
		"polished_granite_slab",
		"andesite_slab",
		"polished_andesite_slab",
		"diorite_slab",
		"polished_diorite_slab",
		"sandstone_slab",
		"smooth_sandstone_slab",
		"red_nether_brick_slab",
		"nether_brick_slab",
		"prismarine_slab",
		"prismarine_brick_slab",
		"dark_prismarine_slab",

		"soul_torch",
		"iron_bars",
		"nether_portal",
		"ender_chest",
		"cake",
		"end_portal",
		"end_portal_frame",
		"composter",
		"cauldron",
		"bell",
		"sweet_berry_bush",
		"torch",
		"glass",
		"glass_pane",
		"cactus",
		"ladder",
		"bed",
		"chest",

		"spruce_leaves",
		"acacia_leaves",
		"dark_oak_leaves",
		"birch_leaves",
		"jungle_leaves",
		"oak_leaves",

		"water",
		"air"
	};
	// let common block ids clump together
	// (also why nonsolid_blocks has more common stuff last)
	static constexpr std::array<const char*, 13> common_blocks{
		"removed", // solid blocks completely surrounded by other solid blocks
		"stone",
		"bedrock",
		"grass_block",
		"dirt",
		"sand",
		"gravel",
		"diorite",
		"granite",
		"andesite",
		"sandstone",
		"red_sand",
		"terracotta"
	};
	for (const char* k : nonsolid_blocks) {
		p[k] = next++;
	}
	for (const char* k : common_blocks) {
		p[k] = next++;
	}

	air = p["air"];
	p["cave_air"] = air;

	removed = p["removed"];

	requested.insert("removed");
	requested.insert("air");
}
