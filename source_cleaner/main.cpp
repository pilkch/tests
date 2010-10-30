// This program searches for SOURCE_FILES recursively in the the specified directory and then performs various    
// operations on them.  If no directory is specified the current directory is assumed.
//
// SOURCE_FILES are currently:
// *.txt, *.cpp, *.h, *.html, *.htm, *.xml

// Operations:
// Replace tabs with spaces
// Remove trailing spaces
// Remove trailing new lines
// Make sure that there is a newline at the end of the file

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#if defined(__LINUX__) || defined(__APPLE__)
#define BUILD_LINUX_OR_UNIX
#endif

#define nullptr NULL

// Just an arbitrary number
const size_t MAX_PATH_LEN = 512;

// Adjust these to your personal preferences
const size_t nSpacesInEachTab = 2;

std::string GetCurrentDirectory()
{
  char szDirectory[MAX_PATH_LEN];
  getcwd(szDirectory, MAX_PATH_LEN);
  return std::string(szDirectory);
}

bool GetEnvironmentVariable(const std::string& sEnvironmentVariable, std::string& sValue)
{
  sValue.clear();

  const char* szResult = getenv(sEnvironmentVariable.c_str());
  if (szResult == nullptr) return false;

  sValue = szResult;
  return true;
}

std::string GetTempDirectory()
{
#ifdef P_tmpdir
  // On some systems this is defined for us
  return P_tmpdir;
#endif

  std::string sPath;
  if (GetEnvironmentVariable("TMPDIR", sPath)) return sPath;

  // Last resort
  return "/tmp";
}

void CopyContentsOfFile(const std::string& sFrom, const std::string& sTo)
{
  std::ifstream i(sFrom.c_str());
  std::ofstream o(sTo.c_str());

  char c = '\0';

  while (i.get(c)) o.put(c);
}

void DeleteFile(const std::string& sPath)
{
  unlink(sPath.c_str());
}

bool IsDirectory(const std::string& sPath)
{
#ifdef __WIN__
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind = FindFirstFile(sPath.c_str(), &FindFileData);
  if (hFind != INVALID_HANDLE_VALUE) {
    FindClose(hFind);
    return true;
  }

  return false;
#elif defined(BUILD_LINUX_OR_UNIX)
  struct stat _stat;
  int result = lstat(sPath.c_str(), &_stat);
  if (0 > result) {
    std::cout<<"path::IsDirectory lstat FAILED returned "<<result<<" for file "<<sPath<<std::endl;
    return false;
  }
  return S_ISDIR(_stat.st_mode);
#else
#error "path::IsDirectory not implemented on this platform"
#endif
}

std::string GetExtension(const std::string& sFilename)
{
  std::string s = sFilename;

  std::string::size_type i = s.find("/");;
  while(i != std::string::npos) {
    i++;
    s = s.substr(i);
    i = s.find("/");
  };

  if (i != std::string::npos) s = s.substr(i);

  i = s.find(".");
  while (i != std::string::npos) {
    i++;
    s = s.substr(i);
    i = s.find(".");
  };

  if (i != std::string::npos) return s.substr(i);

  return s.substr(0);
}



class cFileCleaner
{
public:
  cFileCleaner();
  ~cFileCleaner();

  void Clean(const std::string& sFilename) const;

private:
  std::string ConvertTabsToSpaces(const std::string& sLine) const;
  std::string RemoveTrailingSpaces(const std::string& sLine) const;
  void RemoveTrailingNewLines(std::vector<std::string>& vLines) const;
  void AddNewLineAtEndOfFile(std::vector<std::string>& vLines) const;

   bool Read(std::ifstream& i, std::vector<std::string>& vLines) const;
   bool Write(std::ofstream& o, const std::vector<std::string>& vLines) const;

  std::string sTempFilePath;
};

cFileCleaner::cFileCleaner()
{
  srand(time(NULL));

  std::string sRandomString;
  const size_t n = 32;
  sRandomString.reserve(n);
  for (size_t i = 0; i < n; i++) sRandomString += 'a' + (rand() % 26);

  sTempFilePath = GetTempDirectory() + "/" + sRandomString + ".txt";

  //std::cout<<"cFileCleaner::cFileCleaner "<<sTempFilePath<<std::endl;
}

cFileCleaner::~cFileCleaner()
{
  DeleteFile(sTempFilePath);
}

void cFileCleaner::Clean(const std::string& sFilename) const
{
  //std::cout<<"cFileCleaner::Clean "<<sTempFilePath<<std::endl;

  {
    std::ifstream i(sFilename.c_str());

    std::vector<std::string> vLines;
    if (!Read(i, vLines)) return;

      const size_t nLines = vLines.size();
      for (size_t iLine = 0; iLine < nLines; iLine++) {
         vLines[iLine] = ConvertTabsToSpaces(vLines[iLine]);
         vLines[iLine] = RemoveTrailingSpaces(vLines[iLine]);
      }

    RemoveTrailingNewLines(vLines);
    AddNewLineAtEndOfFile(vLines);

    // If the last line is a lonely new line character then remove it because it will be added in later anyway by the Write function
    const size_t n = vLines.size();
    if (n != 0) {
      if (vLines[n - 1] == "\n") vLines.pop_back();
    }

    std::ofstream o(sTempFilePath.c_str());
    if (!Write(o, vLines)) return;
  }

  // Now read the whole file from the temp directory back over the original file
  CopyContentsOfFile(sTempFilePath, sFilename);
}

std::string cFileCleaner::ConvertTabsToSpaces(const std::string& sLine) const
{
   std::ostringstream o;

   const size_t n = sLine.length();
   for (size_t i = 0; i < n; i++) {
      const char c = sLine[i];
      if (c == '\t') {
         // Replace each tab with the number of spaces that we like
         for (size_t iSpaces = 0; iSpaces < nSpacesInEachTab; iSpaces++) o.put(' ');
      } else o.put(c);
   }

   return o.str();
}

std::string cFileCleaner::RemoveTrailingSpaces(const std::string& sLine) const
{
   //std::cout<<"cFileCleaner::RemoveTrailingSpaces"<<std::endl;
   std::string o(sLine);
   size_t i = o.find_last_not_of(" \r\n\t");
   if ((i == std::string::npos) || (o[i] == ' ') || (o[i] == '\r') || (o[i] == '\n') || (o[i] == '\t')) {
      //std::cout<<"cFileCleaner::RemoveTrailingSpaces Trailing whitespace not found, returning \"\""<<std::endl;
      return "";
   }

   //std::cout<<"cFileCleaner::RemoveTrailingSpaces Found trailing spaces in line \""<<o<<"\""<<std::endl;
   o.erase(i + 1);

   //std::cout<<"cFileCleaner::RemoveTrailingSpaces returning \""<<o<<"\""<<std::endl;
   return o;
}

void cFileCleaner::RemoveTrailingNewLines(std::vector<std::string>& vLines) const
{
   //std::cout<<"cFileCleaner::RemoveTrailingNewLines"<<std::endl;

   const size_t n = vLines.size();
   if (n == 0) {
      //std::cout<<"cFileCleaner::RemoveTrailingNewLines No lines, returning"<<std::endl;
      return;
   }

   size_t i = n;
   size_t iErase = 0;

   std::vector<std::string>::reverse_iterator iter(vLines.rbegin());
   std::vector<std::string>::reverse_iterator iterEnd(vLines.rend());
   while ((iter != iterEnd) && ((*iter).empty() || ((*iter) == "\r") || ((*iter) == "\n") || ((*iter) == "\r\n"))) {
      assert(i != 0);
      if (i != 0) i--;
      iErase++;
      iter++;
   }

   // If we are at the end of the vector then we have to return
   if (iErase == 0) return;

   // Erase everything from i onwards
   //std::cout<<"cFileCleaner::RemoveTrailingNewLines Non empty line found at position "<<i<<std::endl;
   vLines.erase(vLines.begin() + i, vLines.begin() + i + iErase);
}

void cFileCleaner::AddNewLineAtEndOfFile(std::vector<std::string>&) const
{
   // Because of how we are writing the lines to the file we don't have to do this
   /*const size_t n = vLines.size();
   if (n != 0) {
      // If the last line is not a lonely new line character then add one
      if (vLines[n - 1] != "\n") vLines.push_back("\n");
   }*/
}

bool cFileCleaner::Read(std::ifstream& i, std::vector<std::string>& vLines) const
{
   vLines.clear();

   if (!i) {
      std::cerr<<"Input file not open."<<std::endl;
      return false;
   }

   std::string sLine;
   while (std::getline(i, sLine)) vLines.push_back(sLine);

   // if reason of termination != eof
   if (!i.eof()) std::cerr<<"Error while parsing file."<<std::endl;

   return true;
}

bool cFileCleaner::Write(std::ofstream& o, const std::vector<std::string>& vLines) const
{
   if (!o) {
      std::cerr<<"Output file not open."<<std::endl;
      return false;
   }

   // copy algorithm with ostream_iterator
   std::copy(vLines.begin(), vLines.end(), std::ostream_iterator<std::string>(o, "\n"));

   return true;
}

void PrintUsage(const std::string& sExecutableName)
{
  std::cout<<"Usage: "<<sExecutableName<<" [DIRECTORY]"<<std::endl;
  std::cout<<"Search recursively in DIRECTORY for *.txt, *.cpp, *.h, *.html files to clean up"<<std::endl;
  std::cout<<"Cleaning up involves replacing tabs with 2 spaces"<<std::endl;
  std::cout<<"If no directory is specified the current directory is searched"<<std::endl;
}

class cDirectoryReader
{
public:
  explicit cDirectoryReader(const std::string& sDirectory) { ReadDirectory(sDirectory); }
  ~cDirectoryReader() { Clear(); }

  const std::string& GetFullPath() const { return sPath; }

  const std::vector<std::string>& GetFiles() const { return files; }
  const std::vector<cDirectoryReader*>& GetDirectories() const { return directories; }

private:
  void ReadDirectory(const std::string& sDirectory);
  void Clear();

  std::string sPath;
  std::vector<std::string> files;
  std::vector<cDirectoryReader*> directories;
};

void cDirectoryReader::ReadDirectory(const std::string& sDirectory)
{
  Clear();

  DIR* dp = opendir(sDirectory.c_str());
  if (dp == NULL) {
    std::cerr<<"Error(" << errno << ") opening "<<sDirectory<<std::endl;
    return;
  }

  sPath = sDirectory;

  struct dirent* dirp;
  while ((dirp = readdir(dp)) != NULL) {
    // If we don't have a hidden directory, including "." and "..", then add it to the list
    if (dirp->d_name[0] != '.') {
      const std::string sFullPath(sDirectory + "/" + std::string(dirp->d_name));
      if (IsDirectory(sFullPath)) {
        // We have a directory, create a new directory node and then parse inside there
        cDirectoryReader* pChild = new cDirectoryReader(sFullPath);
        directories.push_back(pChild);
      } else files.push_back(sFullPath);
    }
  }

  closedir(dp);
}

void cDirectoryReader::Clear()
{
  sPath = "";
  files.clear();

  const size_t n = directories.size();
  for (size_t i = 0; i < n; i++) {
    delete directories[i];
  }

  directories.clear();
}

void ProcessDirectory(const cDirectoryReader& dir, cFileCleaner& cleaner)
{
  //std::cout<<"Directory \""<<dir.GetFullPath()<<"\""<<std::endl;

  {
    const std::vector<std::string>& files = dir.GetFiles();
    const size_t n = files.size();
    for (size_t i = 0; i < n; i++) {
      std::string extension(GetExtension(files[i]));
      if (
        (extension == "txt") ||
        (extension == "cpp") ||
        (extension == "h") ||
        (extension == "html") ||
        (extension == "htm") ||
        (extension == "xml")
        ) cleaner.Clean(files[i]);
    }
  }

  {
    const std::vector<cDirectoryReader*>& directories = dir.GetDirectories();
    const size_t n = directories.size();
    for (size_t i = 0; i < n; i++) {
      ProcessDirectory(*directories[i], cleaner);
    }
  }
}

int main(int argc, char** argv)
{
  std::string sDirectory(GetCurrentDirectory());

  if (argc == 2) {
    std::string sArgument1(argv[1]);

    if (sArgument1 == "--help") {
      PrintUsage(argv[0]);
      return EXIT_FAILURE;
    }

    // Else set the directory to the first argument
    sDirectory = sArgument1;
  } else if (argc != 1) {
    // Incorrect number of arguments
    PrintUsage(argv[0]);
    return EXIT_FAILURE;
  }

  cFileCleaner cleaner;

  cDirectoryReader dir(sDirectory);
  ProcessDirectory(dir, cleaner);

  return EXIT_SUCCESS;
}

