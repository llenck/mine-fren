#include <stdio.h>

#include <stdexcept>

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
	{NULL, 0, 0, 0}
};

static void print_help(const char* command) {
	fprintf(stderr, 
	"%s: convert minecraft region files into .seg or .zseg files, precompressing them"
	" for other forms of rendering.\n"
	"\n"
	"Usage: %s [options] <file or directory> ...\n"
	"Where options include:\n"
	"\t-h, --help                   Show this help\n"
	"\t-o, --output-dir             ouput directory for seg/zseg files (default: ./)\n"
	"\t-c, --compress <true|false>  enable/disable compression (default: true)\n"
	"\n"
	"This program is developed at https://github.com/llenck/mcpre-rend (name to"
	" change). If you have any issues, or want to contribute, please go there.\n"
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

static void treat_file(const char* p, int dirfd, ZstdContext& ctx, bool comp) {
	RegionMinifier rm(p, ctx);
	rm.minify_region(comp, dirfd);
}

static void treat_dir(const char* p, int dirfd, ZstdContext& ctx, bool comp) {
	DIR* d = opendir(p);
	if (!d) {
		throw std::runtime_error("Couldn't open directory");
	}

	struct dirent* de;
	while ((de = readdir(d))) {
		if (de->d_type == DT_REG) {
			try {
				treat_file(de->d_name, dirfd, ctx, comp);
			}
			catch (std::logic_error& err) {
				fprintf(stderr, "warning: ignoring logic error on %s\n", de->d_name);
			}
		}
	}

	// possible memory leak: closedir(d) not called if trea_file throws something
	// that is not an std::logic_error
	closedir(d);
}

int main(int argc, char** argv) {
	const char* out_dir = nullptr;
	bool should_compress = true;
	int comp_level = 12;

	while (1) {
		int opt = getopt_long(argc, argv, "ho:c:", opts, NULL);
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

	for (int i = optind; i < argc; i++) {
		const char* p = argv[i];
		mode_t type = path_type(p);

		// we don't need to worry about symlinks; stat(2) dereferences them.
		try {
			if (S_ISREG(type)) {
				treat_file(p, dirfd, ctx, should_compress);
			}
			else if (S_ISDIR(type)) {
				treat_dir(p, dirfd, ctx, should_compress);
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
}
