#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iomanip>
#include <string.h>
#include <filesystem>

using namespace std;

string get_hash(const string& filename) {
	const char *fn = filename.c_str();
	EVP_MD_CTX *mdctx;
	    const EVP_MD *md;
	    int inByte = 0;
	    BIO   *bio_in  = NULL;
	    //BIO   *bio_out = NULL;

	    unsigned char md_value[EVP_MAX_MD_SIZE];
	    unsigned char plaintext[EVP_MAX_MD_SIZE];
	    unsigned int md_len;

	    memset(md_value, '\0', EVP_MAX_MD_SIZE);
	    memset(plaintext, '\0', EVP_MAX_MD_SIZE);

	    bio_in  = BIO_new_file(fn, "r");
	    //bio_out = BIO_new_file(argv[3], "wb");

	    md = EVP_get_digestbyname("sha256");

	    mdctx = EVP_MD_CTX_new();
	    EVP_DigestInit_ex(mdctx, md, NULL);

	    while ((inByte = BIO_read(bio_in, plaintext, EVP_MAX_MD_SIZE)) > 0) {
	        EVP_DigestUpdate(mdctx, plaintext, inByte);
	    }

	    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	    EVP_MD_CTX_free(mdctx);

	    std::ostringstream result;
		for (unsigned int i = 0; i < md_len; i++) {
			result << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(md_value[i]);
		}

	    //BIO_write(bio_out, md_value, md_len);
	    BIO_free(bio_in);
	    //BIO_free(bio_out);

	    return result.str();
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
	strftime(creationTime, sizeof(creationTime), "%c", &lt);
	localtime_r(&mt, &lt);
	char modTime[80];
	strftime(modTime, sizeof(modTime), "%c", &lt);

	stats.push_back(creationTime);
	stats.push_back(modTime);
	stats.push_back(to_string(buff.st_size));
}

void getFilesInFolder(const string& folderPath, vector<string>& files) {
    for (const auto& entry : filesystem::recursive_directory_iterator(folderPath))
    	if (entry.is_regular_file())
    		files.push_back(entry.path());
}

int main() {
	bool verbose = true;

	string folder = "/home/evolve/git/hash";

    vector<string> files;
	getFilesInFolder(folder, files);

	vector<string> stats;

    ofstream csvFile(folder + "/hashes.csv");

    csvFile << "Filename,SHA256 Hash,Creation time,Last modification time, Size in bytes" << endl;

    for (const string& file : files) {
        string hash = get_hash(file);
        getFileStats(file, stats);
        csvFile << file << "," << hash << "," << stats[0] << "," << stats[1] << "," << stats[2] <<endl;
        if (verbose) {
        	cout << "file " << file << "\n" << "SHA256 " << hash << "\n" << "creation time " << stats[0] << "\n"
        			<< "modification time " << stats[1] << "\n" << "size " << stats[2] << " bytes\n";
        }
    }

    csvFile .close();

    return 0;
}
