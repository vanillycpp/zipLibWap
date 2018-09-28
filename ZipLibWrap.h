#pragma once

#include <string>
#include <vector>

#include <zip.h>

class ZipLibWrap
{
public:  
  ZipLibWrap(const std::string& file_name);
  ZipLibWrap(void* data, int data_size);
  ~ZipLibWrap();

  bool saveToMem(void* out_data, int out_data_size);
  bool saveToFile(const std::string& file_name);

  bool listFiles(std::vector<std::string>& out_res);
  bool replaceFile(const std::string& file_name, void* data, int size);

  bool isValid();
  int getDataSize();
  const std::string& getLastError();

private:
  bool makeArchFromMem();
  bool writeArchToMem();

  bool loadDataFromFile(const std::string& file_name);
  bool writeDataToFile(const std::string& file_name);

  void freeData();

  zip_source_t *src;
  zip_t *za;
  zip_error_t error;

  void* m_data;
  int m_data_size;

  std::string m_last_error;
};