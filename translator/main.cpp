#include <cassert>
#include <cstdlib>
#include <cstdio>

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include <sstream>

template <typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept
{
  return N;
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

size_t TranslateTitleCase(const std::string& sInput, std::string& sOutput)
{
  // Convert the string to UTF32
  std::wstring sBuffer = UTF8ToUTF32(sInput);

  wchar_t lastCharacter = ' ';

  // Transform the string to lower case
  auto transformation = [&] (wchar_t ch) {
    if (isspace(lastCharacter) && !isspace(ch)) {
      ch = std::use_facet<std::ctype<wchar_t>>(std::locale()).toupper(ch);
    }

    lastCharacter = ch;

    return ch;
  };
  auto cursor = std::transform(sBuffer.begin(), sBuffer.end(), sBuffer.begin(), transformation);

  // Convert the translated string back again
  sOutput = UTF32ToUTF8(sBuffer);

  return sInput.length();
}

size_t TranslateReverse(const std::string& sInput, std::string& sOutput)
{
  // Convert the string to UTF32
  std::wstring sBuffer = UTF8ToUTF32(sInput);

  // Reverse the string
  std::wstring::iterator begin = sBuffer.begin();
  std::wstring::iterator end = sBuffer.end();
  std::reverse(begin, end);

  // Convert the translated string back again
  sOutput = UTF32ToUTF8(sBuffer);

  return sInput.length();
}

size_t TranslateReverseEachWord(const std::string& sInput, std::string& sOutput)
{
  // Convert the string to UTF32
  std::wstring sBuffer = UTF8ToUTF32(sInput);

  std::wstring::iterator it = sBuffer.begin();

  while (it != sBuffer.end()) {
      while (it != sBuffer.end() && (isspace(*it) || ispunct(*it))) {
          ++it;
      }
      auto begin = it;

      while (it != sBuffer.end() && (!isspace(*it) && !ispunct(*it))) {
          ++it;
      }
      auto end = it;

      // if you want to modify original string instead, just do this:
      std::reverse(begin, end);
  }

  // Convert the translated string back again
  sOutput = UTF32ToUTF8(sBuffer);

  return sInput.length();
}



struct cMode {
  const char* szName;
  const char* szDescription;
  const char* szExample;
  const TranslateFunctionPtr pTranslateFunction;
};

const cMode modes[] = {
  { "upper", "Change characters a-z to upper case", "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", &TranslateUpper },
  { "lower", "Change characters A-Z to lower case", "the quick brown fox jumps over the lazy dog.", &TranslateLower },
  { "titlecase", "Change the first character of each word to upper case", "The Quick Brown Fox Jumps Over The Lazy Dog.", &TranslateTitleCase },
  { "reverse", "Reverse the whole string", ".god yzal eht revo spmuj xof nworb kciuq eht", &TranslateReverse },
  { "reversewords", "Reverse each word", "eht kciuq nworb xof spmuj revo eht yzal god.", &TranslateReverseEachWord },
};


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
  std::cout<<"Modes:"<<std::endl;
  const size_t n = countof(modes);
  for (size_t i = 0; i < n; i++) {
    std::cout<<" "<<modes[i].szName<<": "<<modes[i].szDescription<<" (\""<<modes[i].szExample<<"\")"<<std::endl;
  }
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


  TranslateTitleCase("abc def hij", sOutput);
  assert(sOutput == "Abc Def Hij");

  TranslateTitleCase("ABC DEF HIJ", sOutput);
  assert(sOutput == "ABC DEF HIJ");

  TranslateTitleCase("abc Def. hij. klmnOPQrs TUV", sOutput);
  assert(sOutput == "Abc Def. Hij. KlmnOPQrs TUV");


  TranslateReverse("abc def hij", sOutput);
  assert(sOutput == "jih fed cba");

  TranslateReverse("ABC DEF HIJ", sOutput);
  assert(sOutput == "JIH FED CBA");


  TranslateReverseEachWord("abc def hij", sOutput);
  assert(sOutput == "cba fed jih");

  TranslateReverseEachWord("ABC DEF HIJ", sOutput);
  assert(sOutput == "CBA FED JIH");

  TranslateReverseEachWord("abc Def. hij. klmnOPQrs TUV", sOutput);
  assert(sOutput == "cba feD. jih. srQPOnmlk VUT");
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

  const size_t n = countof(modes);
  for (size_t i = 0; i < n; i++) {
    if (sMode == modes[i].szName) {
      pTranslateFunction = modes[i].pTranslateFunction;
      break;
    }
  }

  if (pTranslateFunction == nullptr) {
    // Unknown translation mode, print the usage and exit
    PrintUsage(GetExecutableName(argv[0]));
    return EXIT_FAILURE;
  }

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
