#include <cassert>
#include <cstdlib>
#include <cstdio>

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <map>
#include <string>
#include <sstream>

#include <experimental/string_view>

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
      // Get the start of this word
      while (it != sBuffer.end() && (isspace(*it) || ispunct(*it))) {
          ++it;
      }
      auto begin = it;

      // Get the end of this word
      while (it != sBuffer.end() && (!isspace(*it) && !ispunct(*it))) {
          ++it;
      }
      auto end = it;

      // Reverse this word
      std::reverse(begin, end);
  }

  // Convert the translated string back again
  sOutput = UTF32ToUTF8(sBuffer);

  return sInput.length();
}

// Typoglycemia
// https://en.wikipedia.org/wiki/Typoglycemia
size_t TranslateShuffleMiddleLettersOfEachWord(const std::string& sInput, std::string& sOutput)
{
  // Convert the string to UTF32
  std::wstring sBuffer = UTF8ToUTF32(sInput);

  // Create our RNG
  std::random_device rd;
  std::mt19937 rng(rd());

  std::wstring::iterator it = sBuffer.begin();

  while (it != sBuffer.end()) {
      // Get the start of this word
      while (it != sBuffer.end() && (isspace(*it) || ispunct(*it))) {
          ++it;
      }
      auto begin = it;

      // Get the end of this word
      while (it != sBuffer.end() && (!isspace(*it) && !ispunct(*it))) {
          ++it;
      }
      auto end = it;

      // Reverse this word if it is at least 4 characters long
      if ((end - begin) > 3) {
        // Skip the first and last characters
        begin++;
        end--;

        // Reverse this word
        const std::wstring original(begin, end);

        // Keep shuffle the middle letters until our string isn't the same any more
        while (std::wstring(begin, end) == original) std::shuffle(begin, end, rng);
      }
  }

  // Convert the translated string back again
  sOutput = UTF32ToUTF8(sBuffer);

  return sInput.length();
}

std::map<char, std::string> BuildAsciiToMorseCodeMap()
{
  return std::map<char, std::string>({
    { 'a', ".-" }, { 'A', ".-" },
    { 'b', "-..." }, { 'B', "-..." },
    { 'c', "-.-." }, { 'C', "-.-." },
    { 'd', "-.." }, { 'D', "-.." },
    { 'e', "." }, { 'E', "." },
    { 'f', "..-." }, { 'F', "..-." },
    { 'g', "--." }, { 'G', "--." },
    { 'h', "...." }, { 'H', "...." },
    { 'i', ".." }, { 'I', ".." },
    { 'j', ".---" }, { 'J', ".---" },
    { 'k', "-.-" }, { 'K', "-.-" },
    { 'l', ".-.." }, { 'L', ".-.." },
    { 'm', "--" }, { 'M', "--" },
    { 'n', "-." }, { 'N', "-." },
    { 'o', "---" }, { 'O', "---" },
    { 'p', ".--." }, { 'P', ".--." },
    { 'q', "--.-" }, { 'Q', "--.-" },
    { 'r', ".-." }, { 'R', ".-." },
    { 's', "..." }, { 'S', "..." },
    { 't', "-" }, { 'T', "-" },
    { 'u', "..-" }, { 'U', "..-" },
    { 'v', "...-" }, { 'V', "...-" },
    { 'w', ".--" }, { 'W', ".--" },
    { 'x', "-..-" }, { 'X', "-..-" },
    { 'y', "-.--" }, { 'Y', "-.--" },
    { 'z', "--.." }, { 'Z', "--.." },

    { '0', "-----" },
    { '1', ".----" },
    { '2', "..---" },
    { '3', "...--" },
    { '4', "....-" },
    { '5', "....." },
    { '6', "-...." },
    { '7', "--..." },
    { '8', "---.." },
    { '9', "----." },

    { '.', ".-.-.-" },
    { ',', "--..--" },
    { '?', "..--.." },
    { '\'', ".----." },
    { '!', "-.-.--" },
    { '/', "-..-." },
    { '(', "-.--." },
    { ')', "-.--.-" },
    { '&', ".-..." },
    { ':', "---..." },
    { ';', "-.-.-." },
    { '=', "-...-" },
    { '+', ".-.-." },
    { '-', "-....-" },
    { '_', "..--.-" },
    { '"', ".-..-." },
    { '$', "...-..-" },
    { '@', ".--.-." },

    { ' ', "   " },
  });
}


// To Morse code
// https://en.wikipedia.org/wiki/Morse_code
// NOTE: This only supports A-Z, a-z, 0-9 and some basic punctuation, it doesn't support international characters, prosign, abbreviations, Q codes or other phrases
size_t TranslateToMorseCode(const std::string& sInput, std::string& sOutput)
{
  sOutput.clear();

  const std::map<char, std::string> mAsciiToMorseCode = BuildAsciiToMorseCodeMap();

  bool first = true;

  for (const char& c : sInput) {
    auto iter = mAsciiToMorseCode.find(c);
    if (iter != mAsciiToMorseCode.end()) {
      if (first) first = false;
      else sOutput += " ";
      sOutput += iter->second;
    } else sOutput += c;
  }

  return sInput.length();
}

// From Morse code
// https://en.wikipedia.org/wiki/Morse_code
// NOTE: This only supports A-Z, a-z, 0-9 and some basic punctuation
size_t TranslateFromMorseCode(const std::string& sInput, std::string& sOutput)
{
  sOutput.clear();

  const std::map<char, std::string> mAsciiToMorseCode = BuildAsciiToMorseCodeMap();

  // Now build the morse code to ascii map
  std::map<std::string, char> mMorseCodeToAscii;

  for (const std::pair<char, std::string>& iter : mAsciiToMorseCode) {
    mMorseCodeToAscii[iter.second] = iter.first;
  }

  std::experimental::string_view v = sInput;
  while (!v.empty()) {
    ssize_t firstSpace = v.find(' ');
    if (firstSpace == std::experimental::string_view::npos) {
      // No more spaces, just translate the string and break
      auto iter = mMorseCodeToAscii.find(v.to_string());
      if (iter == mMorseCodeToAscii.end()) {
        // Didn't find the morse code sequence, just output the input string
        sOutput += sInput;
      } else sOutput += iter->second;

      break;
    } else if (firstSpace != 0) {
      const std::string sMorseCode = v.substr(0, firstSpace).to_string();
      auto iter = mMorseCodeToAscii.find(sMorseCode);
      if (iter != mMorseCodeToAscii.end()) {
        sOutput += iter->second;
        v.remove_prefix(firstSpace + 1);
      }
    } else {
      const size_t n = v.length();
      for (size_t i = 0; i < n; i++) {
        if (v[i] == ' ') {
          const std::string sMorseCode = v.substr(0, i).to_string();
          auto iter = mMorseCodeToAscii.find(sMorseCode);
          if (iter != mMorseCodeToAscii.end()) {
            sOutput += iter->second;
            v.remove_prefix(i + 1);
          }
        }
      }
    }
  }

  return sInput.length();
}


const std::string sBase64Characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t TranslateToBase64(const std::string& sInput, std::string& sOutput)
{
  sOutput.clear();

  int val = 0;
  int valb = -6;
  for (const char& c : sInput) {
    val = (val<<8) + c;
    valb += 8;
    while (valb >= 0) {
      sOutput += sBase64Characters[(val>>valb) & 0x3F];
      valb -= 6;
    }
  }

  if (valb > -6) sOutput += sBase64Characters[((val<<8)>>(valb + 8)) & 0x3F];

  // Pad out equals characters to make the text a multiple of 4 bytes
  while (sOutput.length() % 4) sOutput += '=';
}

size_t TranslateFromBase64(const std::string& sInput, std::string& sOutput)
{
  sOutput.clear();

  std::array<int, 256> fullCharacterTable;

  fullCharacterTable.fill(-1);

  for (int i = 0; i < 64; i++) fullCharacterTable[sBase64Characters[i]] = i;

  int val = 0;
  int valb = -8;
  for (const char& c : sInput) {
    if (fullCharacterTable[c] == -1) break;

    val = (val<<6) + fullCharacterTable[c];
    valb += 6;
    if (valb >= 0) {
      sOutput += char((val>>valb) & 0xFF);
      valb -= 8;
    }
  }
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
  { "shufflemiddleletters", "Shuffle the middle letters in each word", "the qciuk bwron fox jmpus oevr the lzay dog.", &TranslateShuffleMiddleLettersOfEachWord },
  { "tomorsecode", "Convert a string to morse code (Very basic implementation, a-z, A-Z, 0-9, some punctuation", "- .... .   --.- ..- .. -.-. -.-   -... .-. --- .-- -.   ..-. --- -..-   .--- ..- -- .--. ...   --- ...- . .-.   - .... .   .-.. .- --.. -.--   -.. --- --.", &TranslateToMorseCode },
  { "frommorsecode", "Convert a string from morse code (Very basic implementation, a-z, A-Z, 0-9, some punctuation", "the quick brown fox jumps over the lazy dog", &TranslateFromMorseCode },
  { "tobase64", "Convert a string to morse code (Very basic implementation, a-z, A-Z, 0-9, some punctuation", "- .... .   --.- ..- .. -.-. -.-   -... .-. --- .-- -.   ..-. --- -..-   .--- ..- -- .--. ...   --- ...- . .-.   - .... .   .-.. .- --.. -.--   -.. --- --.", &TranslateToMorseCode },
  { "frombase64", "Convert a string from morse code (Very basic implementation, a-z, A-Z, 0-9, some punctuation", "the quick brown fox jumps over the lazy dog", &TranslateFromMorseCode },
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
    std::cout<<"  "<<modes[i].szName<<": "<<modes[i].szDescription<<" (\""<<modes[i].szExample<<"\")"<<std::endl;
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


  TranslateShuffleMiddleLettersOfEachWord("I couldn't believe that I could actually understand what I was reading: the phenomenal power of the human mind. According to a research team at Cambridge University, it doesn't matter in what order the letters in a word are, the only important thing is that the first and last letter be in the right place. The rest can be a total mess and you can still read it without a problem. This is because the human mind does not read every letter by itself, but the word as a whole. Such a condition is appropriately called Typoglycemia.", sOutput);
  // It's a bit tricky to test this one without basically writing the function again which I couldn't be bothered doing


  TranslateToMorseCode("sos SOS", sOutput);
  assert(sOutput == "... --- ...     ... --- ...");

  TranslateToMorseCode("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789", sOutput);
  assert(sOutput == ".- -... -.-. -.. . ..-. --. .... .. .--- -.- .-.. -- -. --- .--. --.- .-. ... - ..- ...- .-- -..- -.-- --..     .- -... -.-. -.. . ..-. --. .... .. .--- -.- .-.. -- -. --- .--. --.- .-. ... - ..- ...- .-- -..- -.-- --..     ----- .---- ..--- ...-- ....- ..... -.... --... ---.. ----.");

  TranslateToMorseCode(".,?'!/()&:;=+-_\"$@", sOutput);
  assert(sOutput == ".-.-.- --..-- ..--.. .----. -.-.-- -..-. -.--. -.--.- .-... ---... -.-.-. -...- .-.-. -....- ..--.- .-..-. ...-..- .--.-.");


  std::string sTemp;
  TranslateToMorseCode("sos SOS", sTemp);
  TranslateFromMorseCode(sTemp, sOutput);
  assert(sOutput == "sos sos");

  TranslateToMorseCode("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789", sTemp);
  TranslateFromMorseCode(sTemp, sOutput);
  assert(sOutput == "abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz 0123456789");

  TranslateToMorseCode(".,?'!/()&:;=+-_\"$@", sTemp);
  TranslateFromMorseCode(sTemp, sOutput);
  assert(sOutput == ".,?'!/()&:;=+-_\"$@");


  TranslateToBase64("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789", sOutput);
  assert(sOutput == "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXogQUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVogMDEyMzQ1Njc4OQ==");

  TranslateToBase64("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789", sTemp);
  TranslateFromBase64(sTemp, sOutput);
  assert(sOutput == "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789");
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
