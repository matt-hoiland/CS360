/*
 * mh_utils.h
 *
 *  Created on: Jan 27, 2016
 *      Author: matt
 */

#ifndef MH_UTILS_H_
#define MH_UTILS_H_

#include <sstream>
#include <string>
#include <vector>

#include <cctype>

namespace MH {
std::vector<std::string> split(std::string, std::string);
std::string ltrim(std::string);
std::string rtrim(std::string);
std::string trim(std::string);
std::string iota(int);
}


#endif /* MH_UTILS_H_ */
