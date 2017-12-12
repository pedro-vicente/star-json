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
  h5iterate_t reader;

  std::string json = reader.make_json(fname.c_str());

  std::cout << "\n";
  for (size_t idx_dst = 0; idx_dst < reader.m_datasets.size(); idx_dst++)
  {
    std::cout << reader.m_datasets.at(idx_dst).m_path.c_str() << "\n";
    size_t nbr_attr = reader.m_datasets.at(idx_dst).m_attributes.size();
    for (size_t idx_att = 0; idx_att < nbr_attr; idx_att++)
    {
      hdf_dataset_t attribute = reader.m_datasets.at(idx_dst).m_attributes.at(idx_att);
      std::cout << attribute.m_path.c_str() << "\n";
    }
  }

  std::cout << "\n";
  std::cout << json;
  std::cout << "\n";

  return 0;
}


