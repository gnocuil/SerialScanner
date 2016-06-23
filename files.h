#pragma once

#include <string>
#include <vector>

std::vector<std::string> single(std::string filename, int threshold = -1);

int multiple(std::string dirname, int depth);
