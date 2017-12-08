/////////////////////////////////////////////////////////////////////////////////////////////////////
//Center for Satellite Applications and Research (STAR)
//NOAA Center for Weather and Climate Prediction (NCWCP)
//5830 University Research Court
//College Park, MD 20740
//Purpose: Parse a STAR JSON format file
//see star_json.html for a description of the format
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include "star_json.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "usage : ./star_json <JSON file>" << std::endl;
    exit(1);
  }
  star_json parser;
  if (parser.convert(argv[1]) < 1)
  {
    return 1;
  }
  return 0;
}

