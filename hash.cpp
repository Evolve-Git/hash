#include "hash.h"

const int MAX_ARGS = 5;

using namespace std;

int main(int argc, char *argv[]) {
	bool verbose = false;
	bool cVerb = false;
	int count = 1;
	int modeSet = 0;
	string folderPath;
	string filePath;
	vector<string> extensions;
	string outputFile;

	if (argc > 2){
		while (argv[count]){
			if (strcmp(argv[count], OPT_SAVE) == 0){
				if (modeSet > 0){
					cout << "Operation mode declared twice!\n";
					goto exit;
				}
				else{
					if (argv[count + 1]){
						folderPath = argv[count + 1];
						formatPath(folderPath);
						count += 2;
						modeSet = 1;
					}
					else{
						cout << "No folder provided!\n";
						goto exit;
					}
				}
			}
			else if (strcmp(argv[count], OPT_COMPARE) == 0){
				if (modeSet > 0){
					cout << "Operation mode declared twice!\n";
					goto exit;
				}
				else{
					if (argv[count + 1]){
						folderPath = argv[count + 1];
					}
					else{
						cout << "No folder provided!\n";
						goto exit;
					}
					if (argv[count + 2]){
						filePath = argv[count + 2];
						count += 3;
						modeSet = 2;
					}
					else{
						cout << "No file for comparison provided!\n";
						goto exit;
					}
				}
			}
			else if (strcmp(argv[count], OPT_VERBOSE) == 0){
				verbose = true;
				count += 1;
			}
			else if (strcmp(argv[count], OPT_FULL) == 0){
				cVerb = true;
				count += 1;
			}
			else if (strcmp(argv[count], OPT_HELP) == 0){
				displayHelp();
				goto exit;
			}
			else if (strcmp(argv[count], OPT_EXT) == 0){
				if (argv[count + 1]){
					string ext = argv[count + 1];
					getFileExtensions(ext, extensions);
					count += 2;
				}
				else{
					cout << "Extensions option selected but no extensions provided!\n";
					goto exit;
				}
			}
			else if (strcmp(argv[count], OPT_OUTFILE) == 0){
				if (argv[count + 1]){
					outputFile = argv[count + 1];
					checkOutputFilename(outputFile);
					count += 2;
				}
				else{
					cout << "Output file name option selected but no filename provided!\n";
					goto exit;
				}
			}
			else{
				cout << "Option is not recognized!\n";
				goto exit;
			}
		}

		formatPath(folderPath);
		if (checkFolder(folderPath)){
			cout << "Target path OK.\n";
			if (modeSet == 1){
				saveHash(folderPath, outputFile, verbose, extensions);
			}
			else if (modeSet == 2){
				formatPath(filePath);
				if (checkFile(filePath)){
					cout << "Target file OK.\n";
					compareHashes(folderPath, filePath, outputFile, verbose, cVerb, extensions);
				}
				else{
					cout << "Csv file does not exist.\n";
				}
			}
			else{
				cout << "No operation mode selected!\n";
			}
		}
		else
			cout << "Folder does not exist.\n";
	}
	else
		displayHelp();

	exit:
    return 0;
}
