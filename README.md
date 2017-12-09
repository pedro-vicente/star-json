# HDF5 STAR-JSON 

STAR JSON is a JSON schema that is used to share commonly used scientific data formats, such as HDF5 and netCDF.
<br/>
It is used in client/server applications, where the JSON representation of the HDF5 file is transmitted over the network.
<br/>
A client application has access to remote HDF5 or netCDF data stored a server application. The client extracts metadata and data from the remote files, transmitted in STAR JSON format. 

Dependencies
------------

[CMake](https://cmake.org)
CMake build system

Optional
<br/>

[HDF5](http://www.hdfgroup.org)
HDF5 is a set of software libraries and self-describing, 
machine-independent data format that support the creation, 
access, and sharing of array-oriented scientific data.
<br />

## JSON parsing

https://github.com/vivkin/gason
A C++ JSON parser (included).
<br />

Building from source
------------

Install dependency packages: Debian-based systems (like Ubuntu)
<pre>
sudo apt-get install build-essential
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

CMake build options
------------
If the HDF5, zlib or szip libraries are not found on the default location, they can be set. 
<pre>
cmake .. \
-DHDF5_INCLUDE:PATH=/your/hdf5/include/path \
-DHDF5_LIBRARY=/your/hdf5/library/file/name \
-DZLIB_LIBRARY=/your/zlib/library/file/name \
-DSZIP_LIBRARY=/your/zlib/library/file/name
</pre>

Usage
------------
./star_json 'JSON file in ./data'

STAR_JSON example
------------

<pre>
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
</pre>

Documentation
--------------
[Specification](http://www.space-research.org/blog/star_json.html)

Use case: ATMS HDF5 data
------------
Advanced Technology Microwave Sounder (ATMS)
The Advanced Technology Microwave Sounder (ATMS), a cross-track scanner with 22 channels, provides sounding observations needed to retrieve profiles of atmospheric temperature and moisture for civilian operational weather forecasting as well as continuity of these measurements for climate monitoring purposes. 

https://jointmission.gsfc.nasa.gov/atms.html

atms_reader
------------

atms_reader reads HDF5 ATMS files and saves a relevamt data in STAR JSON format

Usage
------------
./reader_atms <TATMS file> <GATMO file> 

Example
------------

./reader_atms ../atms/TATMS_npp_d20141130_t1817273_e1817589_b16023_c20141201005810987954_noaa_ops.h5 ../atms/GATMO_npp_d20141130_t1817273_e1817589_b16023_c20141201005333390510_noaa_ops.h5
