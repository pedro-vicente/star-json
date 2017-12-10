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
#include "star_dataset.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "usage : ./read_dataset <JSON file>" << std::endl;
  }

  std::string json_file = argv[1];
  std::vector<star_dataset_t> datasets;
  if (read_datasets(json_file.c_str(), datasets) < 0)
  {
    assert(0);
    return 1;
  }
  return 0;
}


