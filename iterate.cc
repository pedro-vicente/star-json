
#include <iostream>
#include <assert.h>
#include "iterate.hh"

std::string get_json_type(size_t datatype_size, H5T_sign_t datatype_sign, H5T_class_t datatype_class);

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
//h5iterate_t::make_json
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string h5iterate_t::make_json(const char* file_name, size_t buf_size)
{
  hid_t fid;
  std::string json;

  //build vector of H5O_info_t
  m_visit.visit(file_name);

  if ((fid = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
  {
    assert(0);
    return json;
  }

  char *buf = NULL;
  buf = (char *)malloc(buf_size * sizeof(char));
  m_builder = new gason::JSonBuilder(buf, buf_size - 1);

  //make root
  m_builder->startObject();

  //recursive iteration starting from root
  if (iterate("/", fid) < 0)
  {

  }

  if (H5Fclose(fid) < 0)
  {

  }

  //end root
  m_builder->endObject();

  if (!m_builder->isBufferAdequate())
  {
    puts("warning: the buffer is small and the output json is not valid.");
    assert(0);
  }

  std::string json_fname(file_name);
  json_fname += ".star.json";
  FILE* fp = fopen(json_fname.c_str(), "w+t");
  fwrite(buf, strlen(buf), 1, fp);
  fclose(fp);

  json = buf;
  free(buf);
  return json;
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

  int start_grp = 0;
  int start_var = 0;

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

      if (!start_grp)
      {
        m_builder->startObject("groups");
      }
      //group name JSON object
      m_builder->startObject(info.name);

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

      //iterate in sub group passing group id for group as parent
      if (do_iterate && iterate(path, gid) < 0)
      {

      }

      if (H5Gclose(gid) < 0)
      {

      }

      free(info.name);

      //end JSON object group name
      m_builder->endObject();
      //end JSON object "groups"
      if (!start_grp)
      {
        m_builder->endObject();
        start_grp = 1;
      }
      break;

      ///////////////////////////////////////////////////////////////////////////////////////
      //H5G_DATASET
      //////////////////////////////////////////////////////////////////////////////////////

    case H5G_DATASET:

      if (!start_var)
      {
        m_builder->startObject("variables");
      }
      //variable name JSON object
      m_builder->startObject(info.name);

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

      ////////////////////////////////////////////////////////////////
      //add the JSON "shape" object, its value is a JSON array with dimensions
      ////////////////////////////////////////////////////////////////

      {
        m_builder->startArray("shape");
        for (int idx = 0; idx < rank; ++idx)
        {
          m_builder->addValue((size_t)dims[idx]);
        }
        m_builder->endArray(); //shape
      }

      ////////////////////////////////////////////////////////////////////
      //add the "type" object, its value is a JSON string with the HDF5 type description
      ////////////////////////////////////////////////////////////////////

      {
        std::string str = get_json_type(datatype_size, datatype_sign, datatype_class);
        m_builder->addValue("type", str.c_str());
      }

      //store dimensions 
      hsize_t nbr_elements = 1;
      std::vector< hsize_t> dim;
      for (int idx = 0; idx < rank; ++idx)
      {
        dim.push_back(dims[idx]);
        nbr_elements *= dims[idx];
      }

      ///////////////////////////////////////////////////////////////////////////////////////
      //memory structures
      ///////////////////////////////////////////////////////////////////////////////////////

      //store a hdf_dataset_t with full path, dimensions and metadata
      hdf_dataset_t *dataset = new hdf_dataset_t(path.c_str(),
        dim,
        datatype_size,
        datatype_sign,
        datatype_class);

      dataset->m_buf = malloc(static_cast<size_t>(datatype_size * nbr_elements));

      ///////////////////////////////////////////////////////////////////////////////////////
      //attributes
      ///////////////////////////////////////////////////////////////////////////////////////

      if (get_attributes(path, did, dataset) < 0)
      {

      }

      m_datasets.push_back(dataset);

      if (H5Dclose(did) < 0)
      {

      }

      ///////////////////////////////////////////////////////////////////////////////////////
      //end JSON
      ///////////////////////////////////////////////////////////////////////////////////////

      //end JSON object variable name
      m_builder->endObject();
      //end JSON object "variables"
      if (!start_var)
      {
        m_builder->endObject();
        start_var = 1;
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

int h5iterate_t::get_attributes(const std::string& path, const hid_t loc_id, hdf_dataset_t *dataset)
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
    std::vector<hsize_t> dim; //dataset dimensions
    for (int idx = 0; idx < rank; ++idx)
    {
      dim.push_back(dims[idx]);
    }

    std::string attr_path = path;
    attr_path += "/";
    attr_path += attr_name;
    std::cout << attr_path << std::endl;

    //store a hdf_dataset_t with full attribute path, dimensions and metadata
    hdf_dataset_t *attribute = new hdf_dataset_t(attr_path.c_str(),
      dim,
      datatype_size,
      datatype_sign,
      datatype_class);

    //store in dataset's list of attributes
    dataset->m_attributes.push_back(attribute);
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//get_json_type
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string get_json_type(size_t datatype_size, H5T_sign_t datatype_sign, H5T_class_t datatype_class)
{
  switch (datatype_class)
  {
    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_STRING
    ///////////////////////////////////////////////////////////////////////////////////////

  case H5T_STRING:
    return "string";
    break;

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_FLOAT
    ///////////////////////////////////////////////////////////////////////////////////////

  case H5T_FLOAT:
    if (sizeof(float) == datatype_size)
    {
      return "float";
    }
    else if (sizeof(double) == datatype_size)
    {
      return "double";
    }
#if H5_SIZEOF_LONG_DOUBLE !=0
    else if (sizeof(long double) == datatype_size)
    {
      return "ldouble";
    }
#endif
    break;

  case H5T_INTEGER:

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_INTEGER
    ///////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_NATIVE_SCHAR H5T_NATIVE_UCHAR
    ///////////////////////////////////////////////////////////////////////////////////////

    if (sizeof(char) == datatype_size)
    {
      if (H5T_SGN_NONE == datatype_sign)
      {
        return "uchar";
      }
      else
      {
        return "schar";
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_NATIVE_SHORT H5T_NATIVE_USHORT
    ///////////////////////////////////////////////////////////////////////////////////////

    else if (sizeof(short) == datatype_size)
    {
      if (H5T_SGN_NONE == datatype_sign)
      {
        return "ushort";
      }
      else
      {
        return "short";
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_NATIVE_INT H5T_NATIVE_UINT
    ///////////////////////////////////////////////////////////////////////////////////////

    else if (sizeof(int) == datatype_size)
    {
      if (H5T_SGN_NONE == datatype_sign)
      {
        return "uint";
      }
      else
      {
        return "int";
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_NATIVE_LONG H5T_NATIVE_ULONG
    ///////////////////////////////////////////////////////////////////////////////////////

    else if (sizeof(long) == datatype_size)
    {
      if (H5T_SGN_NONE == datatype_sign)
      {
        return "long";
      }
      else
      {
        return "ulong";
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_NATIVE_LLONG H5T_NATIVE_ULLONG
    ///////////////////////////////////////////////////////////////////////////////////////

    else if (sizeof(long long) == datatype_size)
    {
      if (H5T_SGN_NONE == datatype_sign)
      {
        return "ullong";
      }
      else
      {
        return "llong";
      }
    }
    break;

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_COMPOUND
    ///////////////////////////////////////////////////////////////////////////////////////

  case H5T_COMPOUND:
    return "compound";
    break;


    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_ENUM
    ///////////////////////////////////////////////////////////////////////////////////////

  case H5T_ENUM:
    return "enum";
    break;

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_ARRAY
    ///////////////////////////////////////////////////////////////////////////////////////

  case H5T_ARRAY:
    return "array";
    break;

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_VLEN
    ///////////////////////////////////////////////////////////////////////////////////////

  case H5T_VLEN:
    return "vlen";
    break;

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_TIME H5T_BITFIELD H5T_OPAQUE
    ///////////////////////////////////////////////////////////////////////////////////////

  case H5T_TIME:
    return "time";
    break;
  case H5T_BITFIELD:
    return "bitfield";
    break;
  case H5T_OPAQUE:
    return "opaque";
    break;

    ///////////////////////////////////////////////////////////////////////////////////////
    //H5T_REFERENCE
    ///////////////////////////////////////////////////////////////////////////////////////

  case H5T_REFERENCE:
    return "reference";
    break;

  default:
    assert(0);
    break;
  }; //switch

  std::string s;
  assert(0);
  return s;
}
