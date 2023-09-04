#include "hash.h"

using namespace std;

int main(int argc, char *argv[]) {
	bool verbose = false;
	bool cVerb = false;
	int count = 1;
	int modeSet = 0;
	string targetPath = "";
	bool isFile = false;
	string compPath = "";
	vector<string> extensions;
	string outputFile = "";

	if (argc > 2){
		while (argv[count]){
			if (strcmp(argv[count], OPT_SAVE) == 0){
				if (modeSet > 0){
					cout << "Operation mode " << RED << "declared twice!" << BLACK << "\n";
					goto exit;
				}
				else{
					count += 1;
					if (argv[count]){
						targetPath = argv[count];
						modeSet = MODE_SAVE;
					}
					else{
						cout << RED << "No folder " << BLACK << "provided!\n";
						goto exit;
					}
				}
			}
			else if (strcmp(argv[count], OPT_COMPARE) == 0){
				if (modeSet > 0){
					cout << "Operation mode " << RED << "declared twice!" << BLACK << "\n";
					goto exit;
				}
				else{
					count += 1;
					if (argv[count]){
						targetPath = argv[count];
					}
					else{
						cout << RED << "No folder " << BLACK << "provided!\n";
						goto exit;
					}
					count += 1;
					if (argv[count]){
						compPath = argv[count];
						modeSet = MODE_COMPARE;
					}
					else{
						cout << RED << "No file " << BLACK << "for comparison provided!\n";
						goto exit;
					}
				}
			}
			else if (strcmp(argv[count], OPT_MATCH) == 0){
				if (modeSet > 0){
					cout << "Operation mode " << RED << "declared twice!" << BLACK << "\n";
					goto exit;
				}
				else{
					count += 1;
					if (argv[count]){
						targetPath = argv[count];
					}
					else{
						cout << RED << "No file " << BLACK << "provided!\n";
						goto exit;
					}
					count += 1;
					if (argv[count]){
						compPath = argv[count];
						modeSet = MODE_MATCH;
					}
					else{
						cout << RED << "No folder " << BLACK << "for matching provided!\n";
						goto exit;
					}
				}
			}
			else if (strcmp(argv[count], OPT_VERBOSE) == 0){
				verbose = true;
			}
			else if (strcmp(argv[count], OPT_FULL) == 0){
				cVerb = true;
			}
			else if (strcmp(argv[count], OPT_HELP) == 0){
				displayHelp();
				goto exit;
			}
			else if (strcmp(argv[count], OPT_EXT) == 0){
				count += 1;
				if (argv[count]){
					string ext = argv[count];
					getFileExtensions(ext, extensions);
				}
				else{
					cout << "Extensions option selected but " << RED <<
							"no extensions provided" << BLACK << "!\n";
					goto exit;
				}
			}
			else if (strcmp(argv[count], OPT_OUTFILE) == 0){
				count += 1;
				if (argv[count]){
					outputFile = argv[count];
					checkOutputFilename(outputFile);
				}
				else{
					cout << "Output file name option selected but " << RED <<
							"no filename provided" << BLACK << "!\n";
					goto exit;
				}
			}
			else{
				cout << "Option is " << RED << "not recognized" << BLACK << "!\n";
				goto exit;
			}
			count+= 1;
		}

		formatPath(targetPath);
		if (checkFile(targetPath)){
			isFile = true;
		}
		if (checkFolder(targetPath) || isFile){
			cout << "Target " << ((isFile) ? "file " : "folder ") <<
				GREEN << "OK" << BLACK <<".\n";
			if (modeSet == MODE_SAVE && !isFile){
				saveHash(targetPath, outputFile, verbose, extensions);
			}
			else if (modeSet == MODE_COMPARE){
				formatPath(compPath);
				if (checkFile(compPath)){
					cout << "Comparison file " << GREEN << "OK" << BLACK <<".\n";
					compareHashes(targetPath, isFile, compPath, outputFile, verbose, cVerb, extensions);
				}
				else{
					cout << "Csv file " << RED << "does not exist" << BLACK << ".\n";
				}
			}
			else if (modeSet == MODE_MATCH && isFile){
				formatPath(compPath);
				cout << "Comparison path " << GREEN << "OK" << BLACK <<".\n";
				getFileVersions(targetPath, compPath, outputFile);
			}
			else if (isFile)
				cout << "Folder " << RED << "does not exist." << BLACK << "\n";
			else
				cout << RED << "No operation mode " << BLACK << "selected!\n";
		}
		else
			cout << ((isFile) ? "File " : "Folder ") << RED << "does not exist." << BLACK << "\n";
	}
	else
		displayHelp();

	exit:
    return 0;
}
