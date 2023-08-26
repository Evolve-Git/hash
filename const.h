const char* DELIMETER = ";";

const char* OPT_SAVE 	= "-s";	//get hashes and save them to a file
const char* OPT_COMPARE = "-c";	//get hashes and compare them to previously saved hashes
const char* OPT_MATCH	= "-m";	//list csv`s where selected file(hash) has a match
const char* OPT_VERBOSE = "-v";	//verbose output
const char* OPT_FULL 	= "-f";	//include passed checks in comparison
const char* OPT_EXT 	= "-e";	//compare only provided file extensions
const char* OPT_OUTFILE = "-o"; //output file name
const char* OPT_HELP	= "-h"; //show help

const char* ST_PASS = "OK";
const char* ST_MISS = "Missing";
const char* ST_PRES = "Present";
const char* ST_HASH = "Hash differs";
const char* ST_NEW  = "New";

const char* CSV = ".csv";

const char* DATE = "DATE";		//search string

const char* GREEN	= "\033[1;32m";	//console green
const char* RED		= "\033[1;31m";	//console red
const char* BLACK	= "\033[0m";	//console black

const int BUFFER_SIZE = 1024;
const int TIME_LENGTH = 80;
const int DATE_LENGTH = 20;

const char* HELP =
		"This program calculates hash values of all files in a selected folder and its subfolders.\n"
		"Parsed data will be saved to hashes-DATE.csv in the program directory.\n"
		"Output file can then be used for comparison.\n"
		"Comparison data will be saved to comparison-DATE.csv in the program directory.\n"
		"General usage is: hashcomp [-option] [argument]\n"
		"The options are:\n"
		"-h to see this screen;\n"
		"-s [path/to/folder] to save hashes;\n"
		"-c [path/to/folder] [path/to/file.csv]\n"
		"to compare hashes from a folder to the ones saved previously;\n"
		"-m [path/to/file] [path/to/folder/with/previously/saved/hashes]\n"
		"to find a matching file(hash) in previously saved hashes;\n"
		"-v to enable verbose mode;\n"
		"-f to include successes in comparison mode;\n"
		"-e [filetype1,filetype2,...] to compare only specified file types;\n"
		"-o [/path/to/file.csv] to specify the output file name.\n"
		"Example 1, saving hashes:\n"
		"hashcomp -s /path/to/folder\n"
		"Example 2, comparing hashes with output to the console:\n"
		"hashcomp -c /path/to/folder /path/to/csv/file.csv -v\n"
		"Example 3, comparing hashes of only .csv and .txt files\n"
		"and saving output to manually selected file:\n"
		"hashcomp -c /path/to/folder /path/to/csv/file.csv -e csv,txt -o /path/to/newfile\n";
