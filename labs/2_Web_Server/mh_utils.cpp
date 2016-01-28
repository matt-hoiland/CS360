#include "mh_utils.h"

std::vector<std::string> MH::split(std::string values, std::string regex) {
	std::vector<std::string> v;

	unsigned int semi;
	std::string s;
	while ((semi = values.find(regex)) != std::string::npos) {
		v.push_back(trim(values.substr(0, semi)));
		values = values.substr(semi + 1);
	}
	v.push_back(trim(values));

	return v;
}

std::string MH::ltrim(std::string s) {
	unsigned int i = 0;
	while (i < s.length() && isspace(s[i])) i++;
	return s.substr(i);
}

std::string MH::rtrim(std::string s) {
	int i = s.length() - 1;
	while (i >= 0 && isspace(s[i])) i--;
	return s.substr(0, i + 1);
}

std::string MH::trim(std::string s) { return ltrim(rtrim(s)); }

std::string MH::iota(int i) {
	std::ostringstream out;
	out << i;
	return out.str();
}
