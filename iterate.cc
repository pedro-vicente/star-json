
#include <iostream>
#include "iterate.hh"

///////////////////////////////////////////////////////////////////////////////////////
//name_type_t
//struct to get name and type of an object
///////////////////////////////////////////////////////////////////////////////////////

struct name_type_t
{
  char *name;
  int type;
};

///////////////////////////////////////////////////////////////////////////////////////
//count_objects_cb
//callback function function for H5Literate
///////////////////////////////////////////////////////////////////////////////////////

static herr_t count_objects_cb(hid_t, const char *, const H5L_info_t *, void *_op_data)
{
  hsize_t *op_data = (hsize_t *)_op_data;

  (*op_data)++;

  return(H5_ITER_CONT);
}

///////////////////////////////////////////////////////////////////////////////////////
//get_name_type_cb
//callback function function for H5Literate
//////////////////////////////////////////////////////////////////////////////////////

static herr_t get_name_type_cb(hid_t loc_id, const char *name, const H5L_info_t *, void *op_data)
{
  H5G_stat_t stat_buf;

  if (H5Gget_objinfo(loc_id, name, 0, &stat_buf) < 0)
  {

  }

  ((name_type_t *)op_data)->type = stat_buf.type;
  ((name_type_t *)op_data)->name = (char *)strdup(name);

  // define H5_ITER_STOP for return. This will cause the iterator to stop 
  return H5_ITER_STOP;
}

///////////////////////////////////////////////////////////////////////////////////////
//h5iterate_t::find_object
//////////////////////////////////////////////////////////////////////////////////////

H5O_info_added_t* h5iterate_t::find_object(haddr_t addr)
{
  for (size_t idx = 0; idx < m_visit.visit_info.size(); idx++)
  {
    if (addr == m_visit.visit_info[idx].oinfo.addr)
    {
      return &(m_visit.visit_info[idx]);
    }
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//h5iterate_t::get_attr_item
/////////////////////////////////////////////////////////////////////////////////////////////////////

hdf_dataset_t* h5iterate_t::get_attr_item(const char* dset_path)
{
  for (size_t idx = 0; idx < m_attributes.size(); idx++)
  {
    if (dset_path == m_attributes[idx].m_path)
    {
      return &(m_attributes[idx]);
    }
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//h5iterate_t::iterate
/////////////////////////////////////////////////////////////////////////////////////////////////////

int h5iterate_t::iterate()
{
  hid_t fid;

  //build vector of H5O_info_t
  m_visit.visit(m_file_name.c_str());

  if ((fid = H5Fopen(m_file_name.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
  {
    return -1;
  }

  if (iterate("/", fid) < 0)
  {

  }

  if (H5Fclose(fid) < 0)
  {

  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//h5iterate_t::iterate
/////////////////////////////////////////////////////////////////////////////////////////////////////

int h5iterate_t::iterate(const std::string& grp_path, const hid_t loc_id)
{
  hsize_t nbr_objects = 0;
  hsize_t index;
  name_type_t info;
  std::string path;
  bool do_iterate;
  size_t datatype_size;
  H5T_sign_t datatype_sign;
  H5T_class_t datatype_class;
  hid_t gid;
  hid_t did;
  hid_t sid;
  hid_t ftid;
  hid_t mtid;
  hsize_t dims[H5S_MAX_RANK];
  int rank;

  if (H5Literate(loc_id, H5_INDEX_NAME, H5_ITER_INC, NULL, count_objects_cb, &nbr_objects) < 0)
  {

  }

  for (hsize_t idx_obj = 0; idx_obj < nbr_objects; idx_obj++)
  {

    index = idx_obj;

    //'index' allows an interrupted iteration to be resumed; it is passed in by the application with a starting point 
    //and returned by the library with the point at which the iteration stopped

    if (H5Literate(loc_id, H5_INDEX_NAME, H5_ITER_INC, &index, get_name_type_cb, &info) < 0)
    {

    }

    // initialize path 
    path = grp_path;
    if (grp_path != "/")
      path += "/";
    path += info.name;

    std::cout << path << std::endl;

    switch (info.type)
    {
      ///////////////////////////////////////////////////////////////////////////////////////
      //H5G_GROUP
      //////////////////////////////////////////////////////////////////////////////////////

    case H5G_GROUP:

      if ((gid = H5Gopen2(loc_id, info.name, H5P_DEFAULT)) < 0)
      {

      }

      H5O_info_t oinfo_buf;
      do_iterate = true;

      //get object info
      if (H5Oget_info(gid, &oinfo_buf) < 0)
      {

      }


      if (oinfo_buf.rc > 1)
      {
        H5O_info_added_t *oinfo_added = find_object(oinfo_buf.addr);

        if (oinfo_added->added > 0)
        {
          //avoid infinite recursion due to a circular path in the file.
          do_iterate = false;
        }
        else
        {
          oinfo_added->added++;
        }
      }

      //iterate in sub group passing QTreeWidgetItem for group as parent
      if (do_iterate && iterate(path, gid) < 0)
      {

      }

      if (get_attributes(path, gid) < 0)
      {

      }

      if (H5Gclose(gid) < 0)
      {

      }

      free(info.name);
      break;

      ///////////////////////////////////////////////////////////////////////////////////////
      //H5G_DATASET
      //////////////////////////////////////////////////////////////////////////////////////

    case H5G_DATASET:

      if ((did = H5Dopen2(loc_id, info.name, H5P_DEFAULT)) < 0)
      {

      }

      if ((sid = H5Dget_space(did)) < 0)
      {

      }

      if ((rank = H5Sget_simple_extent_dims(sid, dims, NULL)) < 0)
      {

      }

      if ((ftid = H5Dget_type(did)) < 0)
      {

      }

      if ((mtid = H5Tget_native_type(ftid, H5T_DIR_DEFAULT)) < 0)
      {

      }

      ///////////////////////////////////////////////////////////////////////////////////////
      //store datatype sizes and metadata needed to display HDF5 buffer data
      ///////////////////////////////////////////////////////////////////////////////////////

      if ((datatype_size = H5Tget_size(mtid)) == 0)
      {

      }

      if ((datatype_sign = H5Tget_sign(mtid)) < 0)
      {

      }

      if ((datatype_class = H5Tget_class(mtid)) < 0)
      {

      }

      if (H5Sclose(sid) < 0)
      {

      }

      if (H5Tclose(ftid) < 0)
      {

      }

      if (H5Tclose(mtid) < 0)
      {

      }

      //store dimensions 
      std::vector< hsize_t> dim; //dataset dimensions
      for (int idx = 0; idx < rank; ++idx)
      {
        dim.push_back(dims[idx]);
      }

      //store a hdf_dataset_t with full path, dimensions and metadata
      hdf_dataset_t data_item(m_file_name.c_str(),
        path.c_str(),
        dim,
        datatype_size,
        datatype_sign,
        datatype_class);

      m_datasets.push_back(data_item);

      if (get_attributes(path, did) < 0)
      {

      }

      if (H5Dclose(did) < 0)
      {

      }

      break;
    }

  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//h5iterate_t::get_attributes
// it is assumed that loc_id is either from 
// loc_id = H5Gopen( fid, name);
// loc_id = H5Dopen( fid, name);
// loc_id = H5Topen( fid, name);
///////////////////////////////////////////////////////////////////////////////////////

int h5iterate_t::get_attributes(const std::string& path, const hid_t loc_id)
{
  H5O_info_t oinfo;
  hid_t aid;
  size_t datatype_size;
  H5T_sign_t datatype_sign;
  H5T_class_t datatype_class;
  hid_t sid;
  hid_t ftid;
  hid_t mtid;
  hsize_t dims[H5S_MAX_RANK];
  char attr_name[1024];
  int rank;

  //get object info
  if (H5Oget_info(loc_id, &oinfo) < 0)
  {

  }

  for (hsize_t idx = 0; idx < oinfo.num_attrs; idx++)
  {
    if ((aid = H5Aopen_by_idx(loc_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, idx, H5P_DEFAULT, H5P_DEFAULT)) < 0)
    {

    }

    if (H5Aget_name(aid, (size_t)1024, attr_name) < 0)
    {

    }

    if ((sid = H5Aget_space(aid)) < 0)
    {

    }

    if ((rank = H5Sget_simple_extent_dims(sid, dims, NULL)) < 0)
    {

    }

    if ((ftid = H5Aget_type(aid)) < 0)
    {

    }

    if ((mtid = H5Tget_native_type(ftid, H5T_DIR_DEFAULT)) < 0)
    {

    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //store datatype sizes and metadata needed to display HDF5 buffer data
    ///////////////////////////////////////////////////////////////////////////////////////

    if ((datatype_size = H5Tget_size(mtid)) == 0)
    {

    }

    if ((datatype_sign = H5Tget_sign(mtid)) < 0)
    {

    }

    if ((datatype_class = H5Tget_class(mtid)) < 0)
    {

    }

    if (H5Sclose(sid) < 0)
    {

    }

    if (H5Tclose(ftid) < 0)
    {

    }

    if (H5Tclose(mtid) < 0)
    {

    }

    if (H5Aclose(aid) < 0)
    {

    }

    //store dimensions
    std::vector< hsize_t> dim; //dataset dimensions
    for (int idx = 0; idx < rank; ++idx)
    {
      dim.push_back(dims[idx]);
    }

    //store a hdf_dataset_t with full path, dimensions and metadata
    hdf_dataset_t data_item(m_file_name.c_str(),
      path.c_str(),
      dim,
      datatype_size,
      datatype_sign,
      datatype_class);

    m_attributes.push_back(data_item);

    std::string attr_path = path;
    attr_path += "/";
    attr_path += attr_name;
    std::cout << attr_path << std::endl;
  }

  return 0;
}


