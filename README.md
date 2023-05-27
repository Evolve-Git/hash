# hash
This program gets hash values of all files in a selected folder and its subfolders.  
Output is then saved to a file in current directory, named \"hashes-DATE.csv\"  
Output file can then be used for comparison.  
General usage is: hashcomp [-option] [/path/to/directory]  
The options are:  
"s" to save hashes  
"c" to compare hashes from a folder to the ones saved previously  
add "v" to enable verbose mode  
add "f" to include successes to comparison mode.  
Example 1, saving hashes:  
hashcomp -s /path/to/folder  
Example 2, comparing hashes with output to the console:  
hashcomp -cv /path/to/folder /path/to/csv/file.csv
