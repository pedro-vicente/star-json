//std
#include <assert.h>
//local
#include "hdf5.h"
#include "hdf5_hl.h"
#include "read_atms.hh"

////////////////////////////////////////////////////////////////////////////////////////////////////
//get_nelmts
//utility function to get total number of elements from a HDF5 dataset
/////////////////////////////////////////////////////////////////////////////////////////////////////

size_t get_nelmts(hid_t fid, const char *dataset_name)
{
  int rank;
  hsize_t dims[32];
  H5T_class_t class_id;
  size_t type_size;
  if (H5LTget_dataset_ndims(fid, dataset_name, &rank) < 0)
  {
    assert(0);
    return 0;
  }
  if (H5LTget_dataset_info(fid, dataset_name, dims, &class_id, &type_size) < 0)
  {

  }
  hsize_t nbr_elements = 1;
  for (int idx_buf = 0; idx_buf < rank; idx_buf++)
  {
    nbr_elements *= dims[idx_buf];
  }
  return (size_t)nbr_elements;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//atms_reader_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

atms_reader_t::atms_reader_t()
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-SDR-GEO_All/Longitude
  //DATATYPE  H5T_IEEE_F32BE
  //DATASPACE  SIMPLE ( 12, 96 )
  //tmp2_6 = geo_rec.ALL_DATA.ATMS_SDR_GEO_ALL.LONGITUDE._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  m_latitude = new float[12][96];
  std::fill_n(&m_latitude[0][0], 12 * 96, (float)0);
  m_longitude = new float[12][96];
  std::fill_n(&m_longitude[0][0], 12 * 96, (float)0);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-TDR_All/AntennaTemperature
  //DATASET "AntennaTemperature" 
  //DATATYPE  H5T_STD_U16BE
  //DATASPACE  SIMPLE{ (12, 96, 22) / (H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED) }
  //tmp1_1 = tdr_rec.ALL_DATA.ATMS_TDR_ALL.ANTENNATEMPERATURE._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  //converted to float by factor
  m_antenna_temperature = new float[12][96][22];
  std::fill_n(&m_antenna_temperature[0][0][0], 12 * 96 * 22, (float)0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//atms_reader_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

atms_reader_t::~atms_reader_t()
{
  delete[] m_latitude;
  delete[] m_longitude;
  delete[] m_antenna_temperature;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//atms_reader_t::read_TATMS
/////////////////////////////////////////////////////////////////////////////////////////////////////

int atms_reader_t::read_TATMS(const std::string & fname_TATMS)
{
  hid_t fid;
  size_t idx_buf;
  std::string dataset_name;

  if ((fid = H5Fopen(fname_TATMS.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
  {
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-TDR_All/AntennaTemperatureFactors
  //DATASET "AntennaTemperatureFactors"
  //DATATYPE  H5T_IEEE_F32BE
  //DATASPACE  SIMPLE{ (2) / (H5S_UNLIMITED) }
  //tmp1_2 = tdr_rec.ALL_DATA.ATMS_TDR_ALL.ANTENNATEMPERATUREFACTORS._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  dataset_name = "All_Data/ATMS-TDR_All/AntennaTemperatureFactors";
  float *tmp1_2 = new float[get_nelmts(fid, dataset_name.c_str())];
  if (H5LTread_dataset(fid, dataset_name.c_str(), H5T_NATIVE_FLOAT, tmp1_2) < 0)
  {

  }

  m_atf[0] = tmp1_2[0];
  m_atf[1] = tmp1_2[1];

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-TDR_All/AntennaTemperature
  //DATASET "AntennaTemperature" 
  //DATATYPE  H5T_STD_U16BE
  //DATASPACE  SIMPLE{ (12, 96, 22) / (H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED) }
  //tmp1_1 = tdr_rec.ALL_DATA.ATMS_TDR_ALL.ANTENNATEMPERATURE._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  dataset_name = "All_Data/ATMS-TDR_All/AntennaTemperature";
  uint16_t *tmp1_1 = new uint16_t[get_nelmts(fid, dataset_name.c_str())];
  if (H5LTread_dataset(fid, dataset_name.c_str(), H5T_NATIVE_USHORT, tmp1_1) < 0)
  {

  }

  idx_buf = 0;
  for (size_t idx_row = 0; idx_row < ATMS_nbr_rows; idx_row++)
  {
    for (size_t idx_col = 0; idx_col < ATMS_nbr_cols; idx_col++)
    {
      for (size_t idx_cha = 0; idx_cha < ATMS_nbr_cha; idx_cha++)
      {
        m_antenna_temperature[idx_row][idx_col][idx_cha] = tmp1_1[idx_buf] * m_atf[0];
        idx_buf++;
      }
    }
  }

  if (H5Fclose(fid) < 0)
  {

  }

  delete[] tmp1_1;
  delete[] tmp1_2;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//atms_reader_t::read_GATMO
/////////////////////////////////////////////////////////////////////////////////////////////////////

int atms_reader_t::read_GATMO(const std::string &fname_GATMO)
{
  hid_t fid;
  hid_t did;
  hid_t dcpl_id;
  size_t idx_buf;
  std::string dataset_name;
  H5D_fill_value_t fill_status;
  float rd_fill;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-SDR-GEO_All/Longitude
  //DATATYPE  H5T_IEEE_F32BE
  //DATASPACE  SIMPLE ( 12, 96 )
  //tmp2_6 = geo_rec.ALL_DATA.ATMS_SDR_GEO_ALL.LONGITUDE._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if ((fid = H5Fopen(fname_GATMO.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
  {
    return -1;
  }

  dataset_name = "All_Data/ATMS-SDR-GEO_All/Latitude";

  if ((did = H5Dopen2(fid, dataset_name.c_str(), H5P_DEFAULT)) < 0)
  {

  }

  if ((dcpl_id = H5Dget_create_plist(did)) < 0)
  {

  }

  if (H5Pfill_value_defined(dcpl_id, &fill_status) < 0)
  {

  }

  if (H5Pget_fill_value(dcpl_id, H5T_NATIVE_FLOAT, &rd_fill) < 0)
  {

  }

  if (H5Pclose(dcpl_id) < 0)
  {

  }

  if (H5Dclose(did) < 0)
  {

  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-SDR-GEO_All/Latitude
  //DATATYPE  H5T_IEEE_F32BE
  //DATASPACE  SIMPLE  ( 12, 96 )
  //tmp2_5 = geo_rec.ALL_DATA.ATMS_SDR_GEO_ALL.LATITUDE._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  float *tmp2_5 = new float[get_nelmts(fid, dataset_name.c_str())];
  if (H5LTread_dataset(fid, dataset_name.c_str(), H5T_NATIVE_FLOAT, tmp2_5) < 0)
  {

  }

  idx_buf = 0;
  for (size_t idx_row = 0; idx_row < ATMS_nbr_rows; idx_row++)
  {
    for (size_t idx_col = 0; idx_col < ATMS_nbr_cols; idx_col++)
    {
      m_latitude[idx_row][idx_col] = tmp2_5[idx_buf];
      idx_buf++;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-SDR-GEO_All/Longitude
  //DATATYPE  H5T_IEEE_F32BE
  //DATASPACE  SIMPLE ( 12, 96 )
  //tmp2_6 = geo_rec.ALL_DATA.ATMS_SDR_GEO_ALL.LONGITUDE._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  dataset_name = "All_Data/ATMS-SDR-GEO_All/Longitude";
  float *tmp2_6 = new float[get_nelmts(fid, dataset_name.c_str())];
  if (H5LTread_dataset(fid, dataset_name.c_str(), H5T_NATIVE_FLOAT, tmp2_6) < 0)
  {

  }

  idx_buf = 0;
  for (size_t idx_row = 0; idx_row < ATMS_nbr_rows; idx_row++)
  {
    for (size_t idx_col = 0; idx_col < ATMS_nbr_cols; idx_col++)
    {
      m_longitude[idx_row][idx_col] = tmp2_6[idx_buf];
      idx_buf++;
    }
  }

  if (H5Fclose(fid) < 0)
  {

  }

  delete[] tmp2_5;
  delete[] tmp2_6;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//atms_reader_t::make_json
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string atms_reader_t::make_json(const char* fname)
{
  char *buf = NULL;
  size_t buf_size = 1024 * 1024;
  buf = (char *)malloc(buf_size * sizeof(char));
  gason::JSonBuilder doc(buf, buf_size - 1);
  doc.startObject();//root
  doc.startObject("variables");
  doc.startObject("AntennaTemperature");
  doc.startArray("shape");
  doc.addValue(ATMS_nbr_rows);
  doc.addValue(ATMS_nbr_cols);
  doc.addValue(ATMS_nbr_cha);

  doc.endArray();
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

