// This program creates a function to build a value from obfuscated data
// The idea being that you can use this function to store sensitive data instead of just storing it as raw strings that can be viewed in a hex editor or with the "strings" utility

// Standard headers
#include <cassert>
#include <ctime>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// Boost headers
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#if defined(__LINUX__) || defined(__APPLE__)
#define BUILD_LINUX_OR_UNIX
#endif

// Adjust these to your personal preferences
const size_t nSpacesInEachTab = 2;

namespace string
{
  bool bIsInitCalled = false;

  void Init()
  {
    std::locale::global(boost::locale::generator().generate(""));
    bIsInitCalled = true;
  }

  std::string ToLower(const std::string& sText)
  {
    assert(bIsInitCalled);
    return boost::locale::to_lower(sText);
  }

  std::string ToUpper(const std::string& sText)
  {
    assert(bIsInitCalled);
    return boost::locale::to_upper(sText);
  }
}

unsigned int GetRandomNumber(unsigned int uiMax)
{
  return (rand() % uiMax);
}

void CreateHeaderForNameAndSecret(const std::string& sName, const std::string& sSecret)
{
  srand(time(nullptr));

  std::string sIndent;
  sIndent.append(nSpacesInEachTab, ' ');

  std::ofstream f(("discombobulator_" + string::ToLower(sName) + ".h").c_str());

  f<<"// This header was automatically generate by discombobulator"<<std::endl;
  f<<"// https://github.com/pilkch/tests/tree/master/discombobulator"<<std::endl;
  f<<std::endl;
  f<<"#ifndef DISCOMBOBULATOR_"<<string::ToUpper(sName)<<"_H"<<std::endl;
  f<<"#define DISCOMBOBULATOR_"<<string::ToUpper(sName)<<"_H"<<std::endl;
  f<<std::endl;
  f<<"#include <string>"<<std::endl;
  f<<"#include <sstream>"<<std::endl;
  f<<std::endl;
  f<<"// The point of this function is to avoid storing any sensitive data as plain text in the executable"<<std::endl;
  f<<"// This should be checked, a smart compiler may precompute these values and store them as plain text anyway"<<std::endl;
  f<<"// Note that the function relies on the values wrapping at 255"<<std::endl;
  f<<std::endl;
  f<<"namespace discombobulator"<<std::endl;
  f<<"{"<<std::endl;
  f<<sIndent<<"inline std::string GetSecret"<<sName<<"UTF8()"<<std::endl;
  f<<sIndent<<"{"<<std::endl;
  f<<sIndent<<sIndent<<"std::ostringstream o;"<<std::endl;

  std::vector<uint8_t> secret;
  std::vector<uint8_t> random;

  // Note that the decryption relies on the values wrapping at 255
  const size_t n = sSecret.length();
  for (size_t i = 0; i < n; i++) {
    uint8_t uiRandom = GetRandomNumber(255);
    uint8_t uiSecret = sSecret[i] - uiRandom;
    f<<sIndent<<sIndent<<"o<<char(uint8_t("<<uint32_t(uiSecret)<<") + uint8_t("<<uint32_t(uiRandom)<<"));"<<std::endl;

    // Remember our secret and random numbers for testing
    secret.push_back(uiSecret);
    random.push_back(uiRandom);
  }

  f<<std::endl;
  f<<sIndent<<sIndent<<"return o.str();"<<std::endl;
  f<<sIndent<<"}"<<std::endl;
  f<<"}"<<std::endl;
  f<<std::endl;
  f<<"#endif // DISCOMBOBULATOR_"<<string::ToUpper(sName)<<"_H"<<std::endl;
  f<<std::endl;


  // Test decryption
  std::ostringstream o;
  for (size_t i = 0; i < n; i++) o<<char(uint8_t(secret[i]) + uint8_t(random[i]));

  // Make sure that we can decrypt this data and get the same input string
  std::cout<<"Input \""<<sSecret<<"\""<<std::endl;
  std::cout<<"Output \""<<o.str()<<"\""<<std::endl;
  assert(o.str() == sSecret);
}

void PrintUsage(const std::string& sExecutableName)
{
  std::cout<<"Usage: "<<sExecutableName<<" VARIABLE VALUE"<<std::endl;
  std::cout<<"Given a variable and a string it will create a header with a function that can be included in your project to store and decrypt the string"<<std::endl;
}

int main(int argc, char** argv)
{
  string::Init();

  std::string sName;
  std::string sSecret;

  if (argc == 3) {
    sName = argv[1];
    sSecret = argv[2];
  } else {
    // Either "--help" or incorrect number of arguments, either way we just want to print the usage and exit
    PrintUsage(argv[0]);
    return EXIT_FAILURE;
  }

  CreateHeaderForNameAndSecret(sName, sSecret);

  return EXIT_SUCCESS;
}

