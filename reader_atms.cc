#include <iostream>
#include <stdio.h>
#include "read_atms.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
//read Advanced Technology Microwave Sounder(ATMS) HDF5 files
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  std::string fname_TATMS;
  std::string fname_GATMO;
  atms_reader_t reader;

  if (argc != 3)
  {
    std::cout << "usage : ./reader_atms <TATMS file> <GATMO file>" << std::endl;
    exit(1);
  }

  fname_TATMS = argv[1];
  fname_GATMO = argv[2];

  if (reader.read_TATMS(fname_TATMS) < 0)
  {
    return 1;
  }

  if (reader.read_GATMO(fname_GATMO) < 0)
  {
    return 1;
  }

  std::string fname(fname_TATMS);
  fname += ".star.json";
  reader.make_json(fname.c_str());

  return 0;
}


