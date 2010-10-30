#include <iostream>

int main(int argc, char* argv[])
{
  int* int_ptr = NULL;
  void* void_ptr = NULL;
  int (*funct_ptr)(void) = NULL;

  std::cout<<"sizeof(bool):        "<<sizeof(bool)<<" bytes"<<std::endl;
  std::cout<<"sizeof(char):        "<<sizeof(char)<<" bytes"<<std::endl;
  std::cout<<"sizeof(short):       "<<sizeof(short)<<" bytes"<<std::endl;
  std::cout<<"sizeof(int):         "<<sizeof(int)<<" bytes"<<std::endl;
  std::cout<<"sizeof(long):        "<<sizeof(long)<<" bytes"<<std::endl;
  std::cout<<"sizeof(long long):   "<<sizeof(long long)<<" bytes"<<std::endl;
  std::cout<<"sizeof(size_t):      "<<sizeof(size_t)<<" bytes"<<std::endl;
  std::cout<<"sizeof(fpos_t):      "<<sizeof(fpos_t)<<" bytes"<<std::endl;
  std::cout<<"sizeof(off_t):       "<<sizeof(off_t)<<" bytes"<<std::endl;
  std::cout<<"------------------------------"<<std::endl;
  std::cout<<"sizeof(float):       "<<sizeof(float)<<" bytes"<<std::endl;
  std::cout<<"sizeof(double):      "<<sizeof(double)<<" bytes"<<std::endl;
  std::cout<<"sizeof(long double): "<<sizeof(long double)<<" bytes"<<std::endl;
  std::cout<<"------------------------------"<<std::endl;
  std::cout<<"sizeof(*int):        "<<sizeof(int_ptr)<<" bytes"<<std::endl;
  std::cout<<"sizeof(*void):       "<<sizeof(void_ptr)<<" bytes"<<std::endl;
  std::cout<<"sizeof(*function):   "<<sizeof(funct_ptr)<<" bytes"<<std::endl;
  std::cout<<"------------------------------"<<std::endl;
  std::cout<<"Architecture:        "<<8 * sizeof(void_ptr)<<" bit"<<std::endl;

  return EXIT_SUCCESS;
}

