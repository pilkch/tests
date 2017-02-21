#include <cassert>
#include <cmath>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>

// libxdgmm headers
#include <libxdgmm/libxdgmm.h>

bool RunTest()
{
  if (!xdg::IsInstalled()) {
    std::cout<<"RunTest xdg::IsInstalled FAILED, returning false"<<std::endl;
    return false;
  }

  const char* szHome = getenv("HOME");
  if (szHome == nullptr) {
    std::cout<<"RunTest getenv(\"HOME\") FAILED, returning false"<<std::endl;
    return false;
  }

  const std::string sHome = szHome;

  std::string sHomeDirectory;
  xdg::GetHomeDirectory(sHomeDirectory);
  if (sHomeDirectory != sHome) {
    std::cout<<"RunTest xdg::GetHomeDirectory FAILED, returning false"<<std::endl;
    return false;
  }

  std::string sHomeDataDirectory;
  xdg::GetDataHomeDirectory(sHomeDataDirectory);
  if (sHomeDataDirectory != sHome + "/.local/share") {
    std::cout<<"RunTest xdg::GetDataHomeDirectory FAILED, returning false"<<std::endl;
    return false;
  }

  std::string sHomeConfigDirectory;
  xdg::GetConfigHomeDirectory(sHomeConfigDirectory);
  if (sHomeConfigDirectory != sHome + "/.config") {
    std::cout<<"RunTest xdg::GetConfigHomeDirectory FAILED, returning false"<<std::endl;
    return false;
  }

  return true;
}

int main(int argc, char** argv)
{
  bool bIsSuccess = RunTest();

  std::cout<<"Tests "<<(bIsSuccess ? "Passed" : "Failed")<<std::endl;
  return bIsSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}

