#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <string.h>
#include <filesystem>
#include <algorithm>
#include "sha256.h"

const char* OPT_S = "s";	//get hashes and save them to a file
const char* OPT_V = "v";	//verbose output
const char* OPT_C = "c";	//get hashes and compare them to previously saved hashes
const char* OPT_F = "f";	//include passed checks in comparison

const char* ST_PASS = "OK";
const char* ST_MISS = "Missing";
const char* ST_PRES = "Present";
const char* ST_HASH = "Hash differs";
const char* ST_NEW = "New";

using namespace std;

void displayHelp(){
	cout << "This program gets hash values of all files in a selected folder and its subfolders. \n" <<
			"Output is then saved to a file in current directory, named \"hashes-DATE.csv\"\n" <<
			"Output file can then be used for comparison.\n" <<
			"General usage is: hashcomp [-option] [/path/to/directory]\n" <<
			"The options are:\n\"s\" to save hashes\n\"c\" to compare hashes from a folder to the ones " <<
			"saved previously\nadd \"v\" to enable verbose mode\nadd \"f\" to include successes to comparison mode.\n" <<
			"Example 1, saving hashes:\nhashcomp -s /path/to/folder\n" <<
			"Example 2, comparing hashes with output to the console:\nhashcomp -cv /path/to/folder /path/to/csv/file.csv";
}

void formatPath(string& folderPath){
	folderPath.erase(folderPath.find_last_not_of('/') + 1, std::string::npos );
	if (folderPath[0] == '.'){
		folderPath.replace(0, 1, filesystem::current_path().generic_string());
	}
}

bool checkFolder(const string& folderPath){
	return filesystem::is_directory(folderPath);
}

bool checkFile(const string& filePath){
	return filesystem::is_regular_file(filePath);
}

void getFilesInFolder(const string& folderPath, vector<string>& files) {
    for (const auto& entry : filesystem::recursive_directory_iterator(folderPath))
    	if (entry.is_regular_file())
    		files.push_back(entry.path());
}

void readCsv(const string& filePath, vector<vector<string>>& data) {
	ifstream file(filePath);
	if (file.is_open()) {
		string line;
		while (getline(file, line)) {
			vector<string> row;
			stringstream ss(line);
			string value;
			while (getline(ss, value, ',')) {
				row.push_back(value);
			}
			data.push_back(row);
		}
		file.close();
	}
}

string getHash(const string& filename){
	std::ifstream fin(filename, std::ifstream::binary);
	SHA256 sha256stream;

	while(!fin.eof()) {
		char buff[1024];
		fin.read(buff, 1024);
		std::streamsize bytes = fin.gcount();
	    sha256stream.add(&buff, bytes);
	}

	fin.close();

	return sha256stream.getHash();
}

void getFileStats(const string& filename, vector<string>& stats){
	const char* fn = filename.c_str();
	stats.clear();
	struct stat buff;
	stat(fn, &buff);

	time_t ct = buff.st_ctime;
	time_t mt = buff.st_mtime;
	struct tm lt;
	localtime_r(&ct, &lt);
	char creationTime[80];
	strftime(creationTime, sizeof(creationTime), "%Y-%m-%d %H:%M:%S", &lt);
	localtime_r(&mt, &lt);
	char modTime[80];
	strftime(modTime, sizeof(modTime), "%Y-%m-%d %H:%M:%S", &lt);

	stats.push_back(creationTime);
	stats.push_back(modTime);
	stats.push_back(to_string(buff.st_size));
}

string getDate(){
	time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
	struct tm lt;
	localtime_r(&now, &lt);
	char date[20];
	strftime(date, sizeof(date), "%Y-%m-%d", &lt);
	return date;
}

void stripFolderPath(string& file, const string& folderPath){
	string::size_type dot = file.rfind(folderPath);
    if (dot != string::npos)
       file.erase(0, folderPath.length() + 1);
}

void saveHash(const string& folderPath, bool verbose){
	vector<string> files;
	getFilesInFolder(folderPath, files);

	vector<string> stats;

	string csvPath = filesystem::current_path().generic_string() + "/hashes-" + getDate() + ".csv";
	ofstream csvFile(csvPath);

	csvFile << "Filename,SHA256 Hash,Creation time,Last modification time,Size in bytes" << endl;
	int counter = 0;

	for (const string& file : files) {
		counter++;
		string hash = getHash(file);
		getFileStats(file, stats);
		string tmp = file;
		stripFolderPath(tmp, folderPath);
		csvFile << tmp << "," << hash << "," << stats[0] << "," << stats[1] << "," << stats[2] << endl;
		if (verbose) {
			cout << "file " << file << "\n" << "SHA256 " << hash << "\n" << "creation time " << stats[0] << "\n"
					<< "modification time " << stats[1] << "\n" << "size " << stats[2] << " bytes\n";
		}
		cout << "[" << counter << "/" << files.size() << "] \r";
		cout.flush();
	}
	cout << "Done.                                                      " << endl;
	cout << "Processed data saved to a file:" << csvPath << endl;

	csvFile.close();
}

void compareHashes(const string& folderPath, const string& filePath, bool& verbose, bool& cVerb){
		string status;
		vector<vector<string>> csvIn;
		readCsv(filePath, csvIn);
		string csvPath = filesystem::current_path().generic_string() + "/comparison-" + getDate() + ".csv";
		ofstream csvFile(csvPath);
		csvFile << "Status,Filename" << endl;

		vector<string> files;
		vector<string>::iterator itr;
		getFilesInFolder(folderPath, files);

		for (int i = 1; i < int(csvIn.size()); i++){
			string csvEntry = folderPath + '/' + csvIn[i][0];
			if (checkFile(csvEntry)){
				string hash = getHash(csvEntry);

				itr = std::find(files.begin(), files.end(), csvEntry);

				if (itr != files.end()){
					files.erase(itr);
				}

				if (hash.compare(csvIn[i][1]) == 0){
					if (cVerb){
						csvFile << ST_PASS << "," << csvEntry << endl;
					}
					status = ST_PASS;
				}
				else{
					csvFile << ST_HASH << "," << csvEntry << endl;
					status = ST_HASH;
				}
			}
			else{
				csvFile << ST_MISS << "," << csvEntry << endl;
				status = ST_MISS;
			}
			if (verbose || cVerb){
				if (cVerb && status == ST_PASS){
					cout << "\033[1;32m" + status + "\033[0m" << "  " << csvEntry << "\n";
				}
				else if (status != ST_PASS){
					cout << "\033[1;31m" + status + "\033[0m" << "  " << csvEntry << "\n";
				}
			}
			cout << "[" << i << "/" << csvIn.size() << "] \r";
			cout.flush();
		}

		cout << "Done.                                                      " << endl;

		if (!files.empty()){
			for (const string& file : files){
				csvFile << ST_NEW << "," << file << endl;
				status = ST_NEW;
				cout << "\033[1;32m" + status + "\033[0m" << "  " << file << "\n";
			}
		}

		csvFile.close();

		cout << "Processed data saved to a file:" << csvPath << endl;
}
