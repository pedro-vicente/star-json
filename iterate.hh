#ifndef ITERATE_HPP
#define ITERATE_HPP 1

#include <vector>
#include <string>
#include "hdf5.h"
#include "visit.hh"
#include "jsonbuilder.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//hdf_dataset_t (common definition for HDF5 dataset and attribute)
//a HDF dataset is defined here simply has a:
// 1) location in memory to store dataset data
// 2) an array of dimensions
// 3) size, sign and class of datatype
// the array of dimensions (of HDF defined type ' hsize_t') is defined in iteration
// the data buffer and datatype sizes are stored on per load variable 
// from tree using the HDF API from item input
/////////////////////////////////////////////////////////////////////////////////////////////////////

class hdf_dataset_t
{
public:
  hdf_dataset_t(const char* path, const std::vector< hsize_t> &dim,
    size_t size, H5T_sign_t sign, H5T_class_t datatype_class) :
    m_path(path),
    m_dim(dim),
    m_datatype_size(size),
    m_datatype_sign(sign),
    m_datatype_class(datatype_class)
  {
    m_buf = NULL;
  }

  ~hdf_dataset_t()
  {
    if (m_buf)
    {
      free(m_buf);
    }
  }

  std::string m_path;
  std::vector<hsize_t> m_dim;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //needed to access HDF5 buffer data
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  size_t m_datatype_size;
  H5T_sign_t m_datatype_sign;
  H5T_class_t m_datatype_class;
  void *m_buf;

  std::vector <hdf_dataset_t*> m_attributes;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//h5iterate_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class h5iterate_t
{
public:
  h5iterate_t() :
    m_builder(NULL)
  {
  }

  ~h5iterate_t()
  {
    delete m_builder;
    for (size_t idx_dst = 0; idx_dst < m_datasets.size(); idx_dst++)
    {
      size_t nbr_attr = m_datasets.at(idx_dst)->m_attributes.size();
      for (size_t idx_att = 0; idx_att < nbr_attr; idx_att++)
      {
        delete m_datasets.at(idx_dst)->m_attributes.at(idx_att);
      }
      delete m_datasets.at(idx_dst);
    }
  }

  // iterate
  std::string make_json(const char* file_name, size_t buf_size);

  //data to store
  std::vector <hdf_dataset_t*> m_datasets;

protected:
  //build vector of H5O_info_t
  h5visit_t m_visit;
  H5O_info_added_t* find_object(haddr_t addr);
  int iterate(const std::string& grp_path, const hid_t loc_id);
  int get_attributes(const std::string& path, const hid_t loc_id, hdf_dataset_t *dataset);

  //make JSON
  gason::JSonBuilder *m_builder;
};

#endif