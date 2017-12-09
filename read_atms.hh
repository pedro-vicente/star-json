#ifndef READ_ATMS_HH
#define READ_ATMS_HH 1

//std
#include <string>
//included JSON
#include "jsonbuilder.hpp"
//external
#include "hdf5.h"
#include "hdf5_hl.h"

//Advanced Technology Microwave Sounder(ATMS) uncorrected antenna temperatures.
//ATMS rotates counter - clockwise(w.r.t.the positive velocity
//direction) producing 104 views, with each view taking
//approximately 18 msecs. 96 earth view antenna temperatures are
//reported in the TDR for each of the 22 channels.ATMS rotates
//three times every 8 seconds resulting in three scans for every
//single scan of CrIS.
//granule dimension [12, 96]

const size_t ATMS_nbr_rows = 12;
const size_t ATMS_nbr_cols = 96;
const size_t ATMS_nbr_cha = 22;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//atms_reader_t
//data for one HDF5 ATMS RDR and one ATMS GEO file
/////////////////////////////////////////////////////////////////////////////////////////////////////

class atms_reader_t
{
public:
  atms_reader_t();
  ~atms_reader_t();
  int read_TATMS(const std::string & fname_TATMS);
  int read_GATMO(const std::string & fname_GATMO);
  std::string make_json(const char* fname);

private:

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-SDR-GEO_All/Longitude
  //DATATYPE  H5T_IEEE_F32BE
  //DATASPACE  SIMPLE ( 12, 96 )
  //tmp2_6 = geo_rec.ALL_DATA.ATMS_SDR_GEO_ALL.LONGITUDE._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  float(*m_latitude)[96];
  float(*m_longitude)[96];

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-TDR_All/AntennaTemperature
  //DATASET "AntennaTemperature" 
  //DATATYPE  H5T_STD_U16BE
  //DATASPACE  SIMPLE{ (12, 96, 22) / (H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED) }
  //tmp1_1 = tdr_rec.ALL_DATA.ATMS_TDR_ALL.ANTENNATEMPERATURE._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  float(*m_antenna_temperature)[96][22];

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ///All_Data/ATMS-TDR_All/AntennaTemperatureFactors
  //DATASET "AntennaTemperatureFactors"
  //DATATYPE  H5T_IEEE_F32BE
  //DATASPACE  SIMPLE{ (2) / (H5S_UNLIMITED) }
  //tmp1_2 = tdr_rec.ALL_DATA.ATMS_TDR_ALL.ANTENNATEMPERATUREFACTORS._DATA
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  float m_atf[2];
};



#endif
