#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>

int main(int argc, char** argv)
{
  std::string input = "ABC";
  std::vector<std::string> perms;
  perms.push_back(input);
  std::string::iterator itBegin = input.begin();
  std::string::iterator itEnd = input.end();
  while(std::next_permutation(itBegin, itEnd)) {
    perms.push_back(std::string(itBegin, itEnd));
  }
  std::copy(perms.begin(), perms.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

  // EXIT_FAILURE

  return EXIT_SUCCESS;
}

