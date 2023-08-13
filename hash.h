#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <string.h>
#include <filesystem>
#include <algorithm>
#include "sha256.h"

const char* DELIMETER = ";";

const char* OPT_S = "s";	//get hashes and save them to a file
const char* OPT_V = "v";	//verbose output
const char* OPT_C = "c";	//get hashes and compare them to previously saved hashes
const char* OPT_F = "f";	//include passed checks in comparison
const char* OPT_E = "e";	//compare only provided file extensions

const char* ST_PASS = "OK";
const char* ST_MISS = "Missing";
const char* ST_PRES = "Present";
const char* ST_HASH = "Hash differs";
const char* ST_NEW = "New";

const int BUFFER_SIZE = 1024;
const int TIME_LENGTH = 80;
const int DATE_LENGTH = 20;

using namespace std;

void displayHelp(){
	cout << "This program gets hash values of all files in a selected folder and its subfolders.\n"
			"Output is then saved to a file in current directory, named \"hashes-DATE.csv\"\n"
			"Output file can then be used for comparison.\n"
			"General usage is: hashcomp [-option] [/path/to/directory]\n"
			"The options are:\n"
			"\"s\" to save hashes\n"
			"\"c\" to compare hashes from a folder to the ones saved previously\n"
			"add \"v\" to enable verbose mode\n"
			"add \"f\" to include successes to comparison mode.\n"
			"add \"e\" to compare only cpecified file types, separated by a comma(,).\n"
			"Example 1, saving hashes:\nhashcomp -s /path/to/folder\n"
			"Example 2, comparing hashes with output to the console:\n"
			"hashcomp -cv /path/to/folder /path/to/csv/file.csv\n"
			"Example 3, comparing hashes of only .csv and .txt files:\n"
			"hashcomp -ce /path/to/folder /path/to/csv/file.csv csv,txt\n";
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

void getFileExtensions(string& ext, vector<string>& extensions){
	string tmp;
	stringstream ss(ext);

	while(getline(ss, tmp, ',')){
	    extensions.push_back("." + tmp);
	}
}

bool checkExtensions(string& file, vector<string>& extensions){
	for (int i = 0; i < int(extensions.size()); i++){
		if (filesystem::path(file).extension() == extensions[i])
			return 1;
	}
	return 0;
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
			while (getline(ss, value, *DELIMETER)) {
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
		char buff[BUFFER_SIZE];
		fin.read(buff, BUFFER_SIZE);
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
	char creationTime[TIME_LENGTH];
	strftime(creationTime, sizeof(creationTime), "%Y-%m-%d %H:%M:%S", &lt);
	localtime_r(&mt, &lt);
	char modTime[TIME_LENGTH];
	strftime(modTime, sizeof(modTime), "%Y-%m-%d %H:%M:%S", &lt);

	stats.push_back(creationTime);
	stats.push_back(modTime);
	stats.push_back(to_string(buff.st_size));
}

string getDate(){
	time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
	struct tm lt;
	localtime_r(&now, &lt);
	char date[DATE_LENGTH];
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

	csvFile << "Filename" << DELIMETER << "SHA256 Hash" << DELIMETER << "Creation time" << DELIMETER <<
			"Last modification time" << DELIMETER << "Size in bytes" << endl;
	int counter = 0;

	for (const string& file : files) {
		counter++;
		string hash = getHash(file);
		getFileStats(file, stats);
		string tmp = file;
		stripFolderPath(tmp, folderPath);
		csvFile << tmp << DELIMETER << hash << DELIMETER << stats[0] << DELIMETER <<
				stats[1] << DELIMETER << stats[2] << endl;
		if (verbose) {
			cout << "file " << file << "\n" << "SHA256 " << hash << "\n" << "creation time " << stats[0] <<
					"\n" << "modification time " << stats[1] << "\n" << "size " << stats[2] << " bytes\n";
		}
		cout << "[" << counter << "/" << files.size() << "] \r";
		cout.flush();
	}
	cout << "Done.                                                      " << endl;
	cout << "Processed data saved to a file:" << csvPath << endl;

	csvFile.close();
}

void compareHashes(const string& folderPath, const string& filePath, bool& verbose, bool& cVerb,
		vector<string>& extensions){
	string status;
	vector<vector<string>> csvIn;
	readCsv(filePath, csvIn);
	string csvPath = filesystem::current_path().generic_string() + "/comparison-" + getDate() + ".csv";
	ofstream csvFile(csvPath);
	csvFile << "Status" << DELIMETER << "Filename" << endl;

	vector<string> files;
	vector<string>::iterator itr;
	getFilesInFolder(folderPath, files);

	bool ext = !extensions.empty();

	if (ext){
		vector<vector<string>>::iterator iter = csvIn.begin() + 1;

		while (iter != csvIn.end()){
			if(!checkExtensions(iter->at(0), extensions)){
				iter = csvIn.erase(iter);
			}
			else{
				iter++;
			}
		}

		vector<string>::iterator itera = files.begin();
		while (itera != files.end()){
			if(!checkExtensions(*itera, extensions)){
				itera = files.erase(itera);
			}
			else{
				++itera;
			}
		}
	}

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
					csvFile << ST_PASS << DELIMETER << csvEntry << endl;
				}
				status = ST_PASS;
			}
			else{
				csvFile << ST_HASH << DELIMETER << csvEntry << endl;
				status = ST_HASH;
			}
		}
		else{
			csvFile << ST_MISS << DELIMETER << csvEntry << endl;
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
			csvFile << ST_NEW << DELIMETER << file << endl;
			if (verbose || cVerb){
				status = ST_NEW;
				cout << "\033[1;32m" + status + "\033[0m" << "  " << file << "\n";
			}
		}
	}

	csvFile.close();

	cout << "Processed data saved to a file:" << csvPath << endl;
}
