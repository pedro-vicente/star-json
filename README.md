# HDF5 STAR-JSON 

STAR JSON is a JSON schema that is used to share commonly used scientific data formats, such as HDF5 and netCDF.
<br/>
It is used in client/server applications, where the JSON representation of the HDF5 file is transmitted over the network.
<br/>
A client application has access to remote HDF5 or netCDF data stored a server application. The client extracts metadata and data from the remote files, transmitted in STAR JSON format. 

Format specification:
--------------
http://www.space-research.org/blog/star_json.html

Dependencies
------------

[CMake](https://cmake.org)
CMake build system

<br/>

[HDF5](http://www.hdfgroup.org) (optional)
HDF5 is a set of software libraries and self-describing, 
machine-independent data format that support the creation, 
access, and sharing of array-oriented scientific data.

<br />

The HDF5 Lite interface is used:

<br />

https://support.hdfgroup.org/HDF5/doc/HL/RM_H5LT.html

<br />

JSON parsing
------

https://github.com/vivkin/gason
A C++ JSON parser (included).
<br />

Building from source
------------

Install dependency packages: Debian-based systems (like Ubuntu)
<pre>
sudo apt-get install build-essential

sudo apt-get install cmake

sudo apt-get install libhdf5-serial-dev
sudo apt-get install zlib1g-dev
</pre>

Install dependency packages: For RPM-based systems (like Fedora and CentOS)
<pre>
sudo yum install zlib-devel
sudo yum install hdf5 hdf5-devel
</pre>

Get source:
<pre>
git clone https://github.com/pedro-vicente/star-json.git
</pre>


Building with CMake
------------
On most Unix systems, HDF5 and zlib libraries are found on the default location with
<pre>
cd build
cmake ..
</pre>

The inclusion of HDF5 is optional. 

CMake build options
------------
If the HDF5, zlib or szip libraries are not found on the default location, they can be set. 
The HDF5 High Level library is used (header file hdf5_hl.h)
<pre>
cmake .. \
-DHDF5_INCLUDE:PATH=/your/hdf5/include/path \
-DHDF5_HL_INCLUDE:PATH=/your/hdf5_hl/include/path \
-DHDF5_LIBRARY=/your/hdf5/library/file/name \
-DZLIB_LIBRARY=/your/zlib/library/file/name \
-DSZIP_LIBRARY=/your/zlib/library/file/name
</pre>

Programs included:
------------
make_star
generates a STAR JSON example file

read_star
reads a STAR JSON file, and generates HDF5

read_datasets
reads a a STAR JSON file that contain only  datasets

atms_reader
reads HDF5 ATMS files and saves relevant data in STAR JSON format


STAR-JSON example file
------------

```JSON
{
  "variables":{
    "var_1":{
      "shape":[2,3],
      "type":"float",
      "data":[[1,2,3],[4,5,6]],
      "attributes": {
        "char_att": {
          "shape":[3],
          "type":"char",
          "data":["foo"]
          }
      }
    }
  },
  "groups":{
    "g1":{
      "groups":{
        "g11":{
        }
      }
    },
    "g2":{
      "variables":{
        "var_1":{
          "shape":[2,3],
          "type":"float",
          "data":[[1,null,3],[null,null,6]]
        }
      }
    }
  }
}
```


Use case: ATMS HDF5 data
------------
Advanced Technology Microwave Sounder (ATMS)
<br />
The Advanced Technology Microwave Sounder (ATMS), a cross-track scanner with 22 channels, provides sounding observations needed to retrieve profiles of atmospheric temperature and moisture for civilian operational weather forecasting as well as continuity of these measurements for climate monitoring purposes. 

https://jointmission.gsfc.nasa.gov/atms.html


Usage
------------
./reader_atms 'TATMS file' 'GATMO file' 

Usage example
------------

./reader_atms ../atms/TATMS_npp_d20141130_t1817273_e1817589_b16023_c20141201005810987954_noaa_ops.h5 ../atms/GATMO_npp_d20141130_t1817273_e1817589_b16023_c20141201005333390510_noaa_ops.h5

Code example to generate a STAR-JSON file
------

C++ code to generate a 3D array 

```c++
gason::JSonBuilder doc(buf, buf_size - 1);
doc.startObject();//root
doc.startObject("variables");
doc.startObject("AntennaTemperature");

//////////////////////////////////////////////////////////////////////
//add the "shape" object, its value is a JSON array with dimensions
//write a 3D array with dimensions [2,3,4]
//////////////////////////////////////////////////////////////////////

{
  doc.startArray("shape");
  doc.addValue(2);
  doc.addValue(3);
  doc.addValue(4);
  doc.endArray(); //shape
}

//////////////////////////////////////////////////////////////////////
//add the "type" object, its value is a JSON string with HDF5 type
//////////////////////////////////////////////////////////////////////

{
  doc.addValue("type", "float");
}

//////////////////////////////////////////////////////////////////////
//add the "data" object, its value is a JSON array
//////////////////////////////////////////////////////////////////////

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
```

JSON generated 
------

```JSON
{
  "variables": {
    "AntennaTemperature": {
      "shape": [2, 3, 4],
      "type": "float",
      "data": [
        [
          [0, 1, 2, 3],
          [4, 5, 6, 7],
          [8, 9, 10, 11]
        ],
        [
          [12, 13, 14, 15],
          [16, 17, 18, 19],
          [20, 21, 22, 23]
        ]
      ]
    }
  }
}
```