#include <cstdlib>
#include <cstdio>
#include <iostream>

// Condition to stop processing
void PrintToStringStream(std::ostringstream& o) {}

template <typename H, typename... T>
void PrintToStringStream(std::ostringstream& o, H head, T... t)
{
  // Do something useful with the head
  o<<head;

  // Expand the rest (pass it recursively to PrintToStringStream)
  PrintToStringStream(o, t...);
}

// Log receives a list of arguments. We don't know the count or types of the arguments, so we just get the head and expand the rest
template <typename H, typename... T>
void Log(H head, T... t)
{
  std::ostringstream o;

  // Expand the arguments
  PrintToStringStream(o, head, t...);

  std::cout<<o.str()<<std::endl;
}

void LogTest()
{
  // Outputs "Hola mundo 42 n"
  Log("Hola", " mundo ", 42, ' ', 'n');
}

int main(int argc, char* argv[])
{
  LogTest();

  return EXIT_SUCCESS;
}
