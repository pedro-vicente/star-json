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
//included JSON
#include "jsonbuilder.hpp"

std::string make_json(const char* fname);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  std::cout << "usage : ./star_json <JSON file>" << std::endl;
  std::string json_file = "3d_array.star.json";
  make_json(json_file.c_str());

  if (argc > 1)
  {
    json_file = argv[1];
  }

  star_json parser;
  if (parser.convert(json_file.c_str()) < 1)
  {
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string make_json(const char* fname)
{
  char *buf = NULL;
  size_t buf_size = 1024;
  buf = (char *)malloc(buf_size * sizeof(char));
  gason::JSonBuilder doc(buf, buf_size - 1);
  doc.startObject();//root
  doc.startObject("variables");
  doc.startObject("AntennaTemperature");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //add the "shape" object, its value is a JSON array with dimensions
  //write a 3D array with dimensions [2,3,4]
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  {
    doc.startArray("shape");
    doc.addValue(2);
    doc.addValue(3);
    doc.addValue(4);
    doc.endArray(); //shape
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //add the "type" object, its value is a JSON string with HDF5 type
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  {
    doc.addValue("type", "float");
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //add the "data" object, its value is a JSON array
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  {
    doc.startArray("data");
    size_t idx = 0;
    for (size_t idx_row = 0; idx_row < 2; idx_row++)
    {
      //first dimension
      doc.startArray();
      for (size_t idx_col = 0; idx_col < 3; idx_col++)
      {
        //second dimension
        doc.startArray();
        for (size_t idx_lev = 0; idx_lev < 4; idx_lev++)
        {
          doc.addValue(idx);
          idx++;
        }
        //second dimension
        doc.endArray();
      }
      //first dimension
      doc.endArray();
    }
    doc.endArray(); //data
  }


  doc.endObject(); //AntennaTemperature
  doc.endObject(); //variables
  doc.endObject(); //root

  if (!doc.isBufferAdequate())
  {
    puts("warning: the buffer is small and the output json is not valid.");
    assert(0);
  }

  FILE* fp = fopen(fname, "w+t");
  fwrite(buf, strlen(buf), 1, fp);
  fclose(fp);

  std::string json(buf);
  free(buf);
  return json;
}


