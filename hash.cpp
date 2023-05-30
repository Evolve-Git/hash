#include "hash.h"

using namespace std;

int main(int argc, char *argv[]) {
	bool verbose = false;
	bool cVerb = false;

	if (argc < 2){
		displayHelp();
	}
	else if (argc > 4){
		cout << "Too many arguments.";
	}
	else{
		if (strstr(argv[1], OPT_V) != NULL) verbose = true;
		if (strstr(argv[1], OPT_F) != NULL) cVerb = true;

		string folderPath = argv[2];
		formatPath(folderPath);
		//string folder = "/home/evolve/git/hash";
		if (checkFolder(folderPath)){
			cout << "Path OK.\n";
			if (strstr(argv[1], OPT_S) != NULL){
				saveHash(folderPath, verbose);
			}
			else if (strstr(argv[1], OPT_C) != NULL){
				string filePath = argv[3];
				formatPath(filePath);
				if (checkFile(filePath)){
					cout << "File OK.\n";
					compareHashes(folderPath, filePath, verbose, cVerb);
				}
				else{
					cout << "Csv file not found.";
				}
			}
			else{
				cout << "Unrecognized option.";
			}
		}
		else
			cout << "Unrecognized path.";
	}
    return 0;
}
