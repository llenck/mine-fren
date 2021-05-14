#include <stdio.h>
#include <stdlib.h>

#include <stdexcept>
#include <vector>

#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "minifier.hpp"

static struct option opts[] = {
	{"help", no_argument, 0, 'h'},
	{"output-dir", required_argument, 0, 'o'},
	{"compress", required_argument, 0, 'c'},
	{"compression-level", required_argument, 0, 'l'},
	{NULL, 0, 0, 0}
};

static __attribute__((noreturn)) void print_help(const char* command) {
	fprintf(stderr, 
	"%s: convert minecraft region files into .seg or .zseg files, precompressing them\n"
	" for other forms of rendering.\n"
	"\n"
	"Usage: %s [options] <file or directory> ...\n"
	"Where options include:\n"
	"\t-h, --help                   Show this help\n"
	"\t-o, --output-dir <dir>       ouput directory for seg/zseg files (default: ./)\n"
	"\t-c, --compress <true|false>  enable/disable compression (default: true)\n"
	"\t-l, --compression-level <N>  set ZSTD compression level (1-19, default: 12)\n"
	"\n"
	"Any additional arguments are interpreted as more files / directories to convert.\n"
	"\n"
	"This program is developed at https://github.com/llenck/mcpre-rend (name to"
	" change)\n"
	"If you have any issues, or want to contribute, please go there.\n"
	, command, command);
	exit(0);
}

static mode_t path_type(const char* path) {
	struct stat stats;

	if (stat(path, &stats) != 0) {
		perror("Couldn't stat argument");
		exit(1);
	}
	else {
		return stats.st_mode;
	}
}

static void treat_file(const char* p, int dirfd, ZstdContext& ctx,
		bool comp)
{
	RegionMinifier rm(p, ctx);
	rm.minify_region(comp, dirfd);
}

static void collect_files(std::vector<std::string>& files, const char* p) {
	DIR* d = opendir(p);
	if (!d) {
		throw std::runtime_error("Couldn't opendir(3) directory");
	}

	struct dirent* de;
	while ((de = readdir(d))) {
		if (de->d_type == DT_REG) {
			std::string full_path(p);
			full_path += "/";
			full_path += de->d_name;
			files.push_back(std::move(full_path));
		}
	}

	closedir(d);
}

int main(int argc, char** argv) {
	const char* out_dir = nullptr;
	bool should_compress = true;
	int comp_level = 12;

	while (1) {
		int opt = getopt_long(argc, argv, "ho:c:l:", opts, NULL);
		if (opt == -1)
			break;

		switch (opt) {
			case '?':
			case 'h':
			default:
				print_help(argv[0]);
				break;

			case 'o':
				out_dir = optarg;
				break;

			case 'c': {
				int r = strcmp(optarg, "true");
				if (r == 0) {
					should_compress = true;
					break;
				}
				r = strcmp(optarg, "false");
				if (r == 0) {
					should_compress = false;
					break;
				}
				else {
					print_help(argv[0]);
				}
			}

			case 'l':
				comp_level = atoi(optarg);
				if (comp_level <= 0 || comp_level > 19)
					print_help(argv[0]);

				break;
		}
	}

	int dirfd = AT_FDCWD;
	if (out_dir) {
		dirfd = open(out_dir, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
		if (dirfd < 0) {
			perror("Couldn't open output directory");
			return 1;
		}
	}

	if (optind == argc) {
		// no files / directories given
		print_help(argv[0]);
		return 1;
	}

	ZstdContext ctx(comp_level);

	std::vector<std::string> files;

	for (int i = optind; i < argc; i++) {
		const char* p = argv[i];
		mode_t type = path_type(p);

		// we don't need to worry about symlinks; stat(2) dereferences them.
		try {
			if (S_ISREG(type)) {
				files.emplace_back(p);
			}
			else if (S_ISDIR(type)) {
				collect_files(files, p);
			}
			else {
				fprintf(stderr, "Can't handle file with mode: %zx\n", (size_t)type);
				return 1;
			}
		}
		catch (std::runtime_error& err) {
			fprintf(stderr, "Error treating %s: %s\n", p, err.what());
		}
	}

	int n = files.size();
	for (int i = 0; i < n; i++) {
		fprintf(stderr, "\rfiles done: %d/%d (%.1f%%)", i, n, i * 100 / (double)n);

		std::string& f = files[i];
		try {
			treat_file(f.c_str(), dirfd, ctx, should_compress);
		}
		catch (std::runtime_error& err) {
			fprintf(stderr, "[WARN] treating %s: %s\n", f.c_str(), err.what());
		}
		catch (std::logic_error& err) {
			fprintf(stderr, "[WARN] ignoring logic error on %s: %s\n",
					f.c_str(), err.what());
		}
	}
	fprintf(stderr, "\rfiles done: %d/%d (100.0%%)\n", n, n);
}
