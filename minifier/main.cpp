#include "region_minifier.hpp"

int main() {
	RegionMinifier rm("r.-1.0.mca", ZstdContext(15));

	rm.minify_region();
}
