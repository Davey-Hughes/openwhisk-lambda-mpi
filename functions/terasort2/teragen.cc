#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <filesystem>

#include <climits>
#include <cstdlib>
#include <cstring>

/* #define MAPPER_NUM 100 */
/* #define REDUCER_NUM 100 */
/* #define STR_NUM_PER_MAPPER (1024 * 1024) */
/* #define STR_LEN 32 */

#define BUFSIZE 100
#define ROW_MAX 0xffffffffff
#define KEY_LEN 10
#define ID_LEN 32
#define FILL_LEN 48

/* gives approximately 32MiB files */
#define FILE_NROWS 330000

void
print_usage()
{
	std::cout << "usage: ./teragen <number of 100-byte rows> <output dir>"
		  << std::endl;
}

std::string
gen_key(std::mt19937 &rng)
{
	std::uniform_int_distribution<char> uni(' ', '~');
	std::ostringstream key;
	for (int i = 0; i < KEY_LEN; ++i) {
		key << uni(rng);
	}

	return key.str();
}

std::string
gen_rowid(long long row)
{
	std::ostringstream rowid;
	rowid << std::setfill(' ') << std::setw(ID_LEN) << std::hex << row;

	return rowid.str();
}

std::string
gen_filler(std::mt19937 &rng)
{
	std::uniform_int_distribution<char> uni('A', 'Z');
	std::ostringstream fill;
	for (int i = 0; i < FILL_LEN; ++i) {
		fill << uni(rng);
	}

	return fill.str();
}

std::string
gen_row(std::mt19937 &rng, long long row_num)
{
	std::ostringstream row;
	row << gen_key(rng)
	    << char(0x00) << char(0x11)
	    << gen_rowid(row_num)
	    << char(0x88) << char(0x99) << char(0xaa) << char(0xbb)
	    << gen_filler(rng)
	    << char(0xcc) << char(0xdd) << char(0xee) << char(0xff);

	return row.str();
}

int
main(int argc, char **argv)
{
	if (argc < 3) {
		print_usage();
		exit(1);
	}

	/* get inputs from command line */
	char *end;
	long long nrows = std::strtoll(argv[1], &end, 10);
	if (nrows > ROW_MAX) {
		std::cout << "Largest row count is: " << std::hex
			  << ROW_MAX << std::endl;
		exit(2);
	}

	std::string outputdir(argv[2]);
	if (!std::filesystem::exists(outputdir)) {
		std::cout << "Output directory: " << outputdir
			  << " does not exist" << std::endl;
		exit(3);
	}

	/* initialize rng */
	std::random_device rd;
	std::mt19937 rng(rd());

	/* write the rows into files */
	long long rowid = 0;
	int nfiles = nrows / FILE_NROWS;
	for (int i = 0; i < nfiles + 1; ++i) {
		std::ostringstream ofilename;
		ofilename << outputdir
			  << "/__random_bytes_32M_"
			  << std::setfill('0') << std::setw(4)
			  << i << "__.dat";

		std::ofstream outfile(ofilename.str(), std::ofstream::binary);

		/* generate data for each row */
		while (1) {
			std::string row = gen_row(rng, rowid);
			rowid++;
			outfile << row;

			if (rowid >= nrows || rowid % FILE_NROWS == 0) {
				break;
			}
		}

		outfile.close();
	}

	return 0;
}
