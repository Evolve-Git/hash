This program calculates hash values of all files in a selected folder and its subfolders.  
Parsed data will be saved to hashes-DATE.csv in the program directory.  
Output file can then be used for comparison.  
Comparison data will be saved to comparison-DATE.csv in the program directory.  
General usage is: hashcomp [-option] [argument]  
The options are:  
-h to see this screen;  
-s [path/to/folder] to save hashes;  
-c [path/to/folder] [path/to/file.csv]  
to compare hashes from a folder to the ones saved previously;  
-v to enable verbose mode;  
-f to include successes in comparison mode;  
-e [filetype1,filetype2,...] to compare only specified file types;  
-o [/path/to/file.csv] to specify the output file name.  
Example 1, saving hashes:  
hashcomp -s /path/to/folder  
Example 2, comparing hashes with output to the console:  
hashcomp -c /path/to/folder /path/to/csv/file.csv -v  
Example 3, comparing hashes of only .csv and .txt files  
and saving output to manually selected file:  
hashcomp -c /path/to/folder /path/to/csv/file.csv -e csv,txt -o /path/to/newfile  