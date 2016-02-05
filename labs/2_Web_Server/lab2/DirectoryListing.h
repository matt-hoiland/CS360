/*
 * DirectoryListing.h
 *
 *  Created on: Jan 27, 2016
 *      Author: matt
 */

#ifndef DIRECTORYLISTING_H_
#define DIRECTORYLISTING_H_

#include <sstream>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

class DirectoryListing {
private:
	struct fileInfo {
		bool dir;
		string name;
		int size;
		string date;
	};
	string path;
	string root;
	vector<fileInfo> files;

public:
	DirectoryListing(string root, string path) {
		this->path = path;
		this->root = root;

        DIR* dirp;
        struct dirent* dp;

        dirp = opendir((root + path).c_str());
        while ((dp = readdir(dirp)) != NULL) {
            struct stat filestat;
            string name = dp->d_name;
            cout << dp->d_name << endl;
            string filepath = root + path + (path[path.size() - 1] == '/' ? "" : "/") + name;
            cout << filepath << endl;
            if (stat(filepath.c_str(), &filestat)) {
            	cout << "continue..." << endl;
            	continue;
            }
            cout << "here!" << endl;
            struct fileInfo info;
            info.dir = S_ISDIR(filestat.st_mode);
            info.name = name;
            info.size = filestat.st_size;
            files.push_back(info);
        }
        closedir(dirp);
	}

	string toString() {
		ostringstream out;
		out << "<DOCTYPE html>\n<html>\n<head>\n";
		out << "<title>Index of " << path << "</title>\n";
		out << "<style type=\"text/css\">table {border-collapse: collapse;} td {border-bottom: 1px solid black; padding: 3px 10px;}</style>\n";
		out << "</head>\n</body>\n";
		out << "<h1>Index of " << path << "</h1>\n";
		out << "<table>\n";
		out << "<tr><th>Dir</th><th>Name</th><th>size</th></tr>\n";
		for (struct fileInfo info : files) {
			out << "<tr><td>" << (info.dir ? "dir" : "")<< "</td>";
			out << "<td><a href=\"" << path << (path[path.size() - 1] == '/' ? "" : "/") << info.name << "\">" << info.name << "</a></td>";
			out << "<td>" << info.size << "</td></tr>\n";
		}
		out << "</table>\n</body>\n</html>\n";
		return out.str();
	}
};


#endif /* DIRECTORYLISTING_H_ */
