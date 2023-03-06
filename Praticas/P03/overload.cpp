//
// Tomás Oliveira e Silva, AED, October 2021
//
// example of function overloading (same function name, different argument numbers and/or data types)
//

#include <iostream>

using std::cout; // make this symbol from the std namespace directly visible

void show(const int i)
{
  cout << "int: "
       << i
       << std::endl;
}

void show(const double d)
{
  cout << "double: "
       << d
       << std::endl;
}

void show(const char *s,const char *h = "string: ") // second argument with default value
{
  cout << h
       << s
       << std::endl;
}

void show(const char c)
{
  cout << "char: "
       << c 
       << std::endl;
}

void show(const int *parray, const int arr_size = 3) //o 5 de la de baixo sobrepoe este 3
{
  int i=0;
  cout << "array: ";
  while (i < arr_size)
  {
    std::cout << parray[i]<<" ";
    i++;
  }
  cout << std::endl;
}

int main(void)
{
  int a[]={25,4,12};
  show(1.0);
  show("hello");
  show(-3);
  show("John","name: ");
  show('a');
  show(a,3);
  return 0;
}
