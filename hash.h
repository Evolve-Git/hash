#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <string.h>
#include <filesystem>
#include <algorithm>
#include "sha256.h"
#include "const.h"

using namespace std;

void displayHelp(){
	cout << HELP << endl;
}

string getDate(){
	time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
	struct tm lt;
	localtime_r(&now, &lt);
	char date[DATE_LENGTH];
	strftime(date, sizeof(date), "%Y-%m-%d", &lt);
	return date;
}

void checkDotPath(string& path){
	if (path[0] == '.'){
		path.replace(0, 1, filesystem::current_path().generic_string());
	}
}

void formatPath(string& folderPath){
	folderPath.erase(folderPath.find_last_not_of('/') + 1, std::string::npos );
	checkDotPath(folderPath);
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

void filterExtensions(vector<string>& files, vector<string>& extensions){
	vector<string>::iterator iter = files.begin();

	while (iter != files.end()){
		if(!checkExtensions(*iter, extensions)){
			iter = files.erase(iter);
		}
		else{
			++iter;
		}
	}
}

void filterCsvExtensions(vector<vector<string>>& csvIn, vector<string>& extensions){
	vector<vector<string>>::iterator iter = csvIn.begin() + 1;

	while (iter != csvIn.end()){
		if(!checkExtensions(iter->at(0), extensions)){
			iter = csvIn.erase(iter);
		}
		else{
			iter++;
		}
	}
}

void ReplaceStringInPlace(string& subject, const string& search, const string& replace) {
    size_t pos = 0;

    while((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

void checkOutputFilename(string& file){
	if (filesystem::path(file).extension() != ".csv")
		file += ".csv";
	checkDotPath(file);
	ReplaceStringInPlace(file, DATE, getDate());
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

void stripFolderPath(string& file, const string& folderPath){
	string::size_type dot = file.rfind(folderPath);

    if (dot != string::npos)
       file.erase(0, folderPath.length() + 1);
}

void saveHash(const string& folderPath, string& outputFile, bool verbose, vector<string>& extensions){
	bool ext = !extensions.empty();
	vector<string> files;

	getFilesInFolder(folderPath, files);

	if (ext){
		filterExtensions(files, extensions);
	}

	vector<string> stats;

	if (outputFile.empty())
		outputFile = filesystem::current_path().generic_string() + "/hashes-" + getDate() + ".csv";
	ofstream csvFile(outputFile);

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
	cout << "Processed data saved to a file:" << outputFile << endl;

	csvFile.close();
}

void compareHashes(const string& folderPath, const string& filePath , string& outputFile, bool& verbose,
		bool& cVerb, vector<string>& extensions){
	bool ext = !extensions.empty();
	string status;
	vector<vector<string>> csvIn;

	readCsv(filePath, csvIn);

	if (outputFile.empty())
		outputFile = filesystem::current_path().generic_string() + "/comparison-" + getDate() + ".csv";
	ofstream csvFile(outputFile);
	csvFile << "Status" << DELIMETER << "Filename" << endl;

	vector<string> files;
	vector<string>::iterator itr;
	getFilesInFolder(folderPath, files);

	if (ext){
		filterCsvExtensions(csvIn, extensions);
		filterExtensions(files, extensions);
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

	cout << "Processed data saved to a file:" << outputFile << endl;
}
