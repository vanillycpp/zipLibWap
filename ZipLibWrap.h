#pragma once

#include <string>
#include <vector>

#include "zipLibInc/zip.h"

class ZipLibWrap
{
public:  
  ZipLibWrap(const std::string& file_name);
  ZipLibWrap(unsigned char* data, int data_size);
  ~ZipLibWrap();

  bool saveToMem(std::vector<unsigned char>& out_data);
  bool saveToFile(const std::string& file_name);

  bool listFiles(std::vector<std::string>& out_res);
  bool replaceFile(const std::string& file_name, void* data, int size);
  bool addFile(const std::string& file_name, void* data, int size);

  bool fileExists(const std::string file_name, bool &out_result);

  bool isValid();
  int getDataSize();
  const std::string& getLastError();

private:
  bool makeArchFromMem();
  //bool writeArchToMem(std::vector<unsigned char>& out_data);

  bool loadDataFromFile(const std::string& file_name, std::vector<unsigned char>& data);
  bool writeDataToFile(const std::string& file_name, const std::vector<unsigned char>& data);

  void freeData();

  zip_source_t *src;
  zip_t *za;
  zip_error_t error;

  void* m_data;
  int m_data_size;

  std::string m_last_error;
};
