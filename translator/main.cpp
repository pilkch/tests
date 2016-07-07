#include <cassert>
#include <cstdlib>
#include <cstdio>

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include <sstream>

std::string GetExecutableName(const char* szArg0)
{
  assert(szArg0 != nullptr);
  
  const char* szAfterLastSlash = szArg0;

  // Get the point just after the last slash
  for (; *szArg0 != 0; szArg0++) {
    switch (*szArg0) {
      case '/':
      case '\\':
        szAfterLastSlash = szArg0 + 1;
    }
  }
  
  return szAfterLastSlash;
}

void PrintUsage(const std::string& sExecutableName)
{
  std::cout<<"Usage: "<<sExecutableName<<" --mode MODE [TEXT]"<<std::endl;
  std::cout<<"Translate a string or stdin"<<std::endl;
  std::cout<<" --mode MODE: Specify which mode to translate with"<<std::endl;
  std::cout<<" TEXT: The text to translate, if this is omitted then the string is read from stdin instead"<<std::endl;
  std::cout<<std::endl;
  std::cout<<"Valid modes:"<<std::endl;
  std::cout<<" upper: Change characters a-z to upper case"<<std::endl;
  std::cout<<" lower: Change characters A-Z to lower case"<<std::endl;
}

std::wstring UTF8ToUTF32(const std::string& sInput)
{
  return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(sInput);
}

std::string UTF32ToUTF8(const std::wstring& sInput)
{
  return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(sInput);
}

// A definition for our translate functions
// They should return the number of characters processed in the input string,
// this allows the calling string to append more characters and do another call if required, to continue the translation where it left off
typedef size_t (*TranslateFunctionPtr)(const std::string& sInput, std::string& sOutput);

size_t TranslateUpper(const std::string& sInput, std::string& sOutput)
{
  // Convert the string to UTF32
  std::wstring sBuffer = UTF8ToUTF32(sInput);

  // Transform the string to lower case
  auto transformation = [] (wchar_t ch) { return std::use_facet<std::ctype<wchar_t>>(std::locale()).toupper(ch); };
  auto cursor = std::transform(sBuffer.begin(), sBuffer.end(), sBuffer.begin(), transformation);

  // Convert the translated string back again
  sOutput = UTF32ToUTF8(sBuffer);

  return sInput.length();
}

size_t TranslateLower(const std::string& sInput, std::string& sOutput)
{
  // Convert the string to UTF32
  std::wstring sBuffer = UTF8ToUTF32(sInput);

  // Transform the string to lower case
  auto transformation = [] (wchar_t ch) { return std::use_facet<std::ctype<wchar_t>>(std::locale()).tolower(ch); };
  auto cursor = std::transform(sBuffer.begin(), sBuffer.end(), sBuffer.begin(), transformation);

  // Convert the translated string back again
  sOutput = UTF32ToUTF8(sBuffer);

  return sInput.length();
}

void TranslateCommandLineArguments(TranslateFunctionPtr pTranslateFunction, const std::string& sInput)
{
  assert(pTranslateFunction != nullptr);

  std::string sOutput;
  (*pTranslateFunction)(sInput, sOutput);
  std::cout<<sOutput;
}

void TranslateFromStandardInput(TranslateFunctionPtr pTranslateFunction)
{
  std::ostringstream o;
  std::string sOutput;

  while (true) {
    const int c = getc(stdin);
    if (c == EOF) break;
    else if (char(c) == '\n') {
      // Translate this line
      (*pTranslateFunction)(o.str(), sOutput);
      std::cout<<sOutput;

      // Go to the next line for consistency
      if (char(c) == '\n') std::cout<<std::endl;

      // Clear the string
      o.str("");
    } else {
      // Just add it to the current line
      assert(c <= 255);
      assert(c >= 0);
      o<<char(c);
    }
  }
}

void UnitTest()
{
  assert(UTF8ToUTF32("abc") == L"abc");
  assert(UTF32ToUTF8(L"abc") == "abc");

  std::string sOutput;
  TranslateUpper("abc", sOutput);
  assert(sOutput == "ABC");
  
  TranslateUpper("ABC", sOutput);
  assert(sOutput == "ABC");
  
  TranslateLower("ABC", sOutput);
  assert(sOutput == "abc");
  
  TranslateLower("abc", sOutput);
  assert(sOutput == "abc");
}

int main(int argc, char* argv[])
{
  UnitTest();

  if (argc < 3) {
    // Either "--help" or incorrect number of arguments, either way we just want to print the usage and exit
    PrintUsage(GetExecutableName(argv[0]));
    return EXIT_FAILURE;
  }

  const std::string sDashMode = argv[1];
  if (sDashMode != "--mode") {
    // First argument wasn't the mode, print the usage and exit
    PrintUsage(GetExecutableName(argv[0]));
    return EXIT_FAILURE;
  }

  const std::string sMode = argv[2];
  TranslateFunctionPtr pTranslateFunction = nullptr;
  if (sMode == "upper") {
    pTranslateFunction = &TranslateUpper;
  } else if (sMode == "lower") {
    pTranslateFunction = &TranslateLower;
  } else {
    // Unknown translation mode, print the usage and exit
    PrintUsage(GetExecutableName(argv[0]));
    return EXIT_FAILURE;
  }

  assert(pTranslateFunction != nullptr);

  if (argc > 3) {
    // Translate arguments
    std::string sInput;
    for (size_t i = 3; i < size_t(argc); i++) {
      if (i == 3) sInput = argv[i];
      else sInput += std::string(" ") + argv[i];
    }
    TranslateCommandLineArguments(pTranslateFunction, sInput);
  } else TranslateFromStandardInput(pTranslateFunction);

  return EXIT_SUCCESS;
}
