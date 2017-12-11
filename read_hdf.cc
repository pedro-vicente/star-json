#include <iostream>
#include <stdio.h>
#include "visit.hh"
#include "iterate.hh"


/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cout << "usage : ./read_hdf <HDF5 file>" << std::endl;
    exit(1);
  }

  std::string fname = argv[1];
  h5iterate_t reader(fname.c_str());

  if (reader.iterate() < 0)
  {
    return 1;
  }

  return 0;
}


