
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ZipLibWrap.h"

ZipLibWrap::ZipLibWrap(const std::string& file_name)
: src(NULL)
, za(NULL)
, m_data(NULL)
, m_data_size(0)
{
  std::vector<unsigned char> data;
  loadDataFromFile(file_name, data);

  if(data.size())
  {
    //m_data = new unsigned char[data.size()];
    m_data = malloc(data.size());
    memcpy(m_data, data.data(), data.size());
    m_data_size = data.size();

    makeArchFromMem();
  }
  else
  {
    m_last_error = "read arh file is empty"; 
  }
}

ZipLibWrap::ZipLibWrap(unsigned char* data, int data_size)
: src(NULL)
, za(NULL)
{
  m_data = malloc(data_size);
  memcpy(m_data, data, data_size);
  m_data_size = data_size;

  makeArchFromMem();
}

ZipLibWrap::~ZipLibWrap()
{
  freeData();  
}

bool ZipLibWrap::replaceFile(const std::string& file_name, void* data, int size)
{
  if(!isValid())
  {
    m_last_error = "archive handle is not valid";
    return false;
  }

  zip_int64_t file_index = zip_name_locate(za, file_name.c_str(), 0);

  if(file_index == -1)
  {
    m_last_error = "can't get arc file index";
    return false;    
  }

  zip_source_t *s; 
   
  if ((s=zip_source_buffer(za, data, size, 0)) == NULL || 
      zip_file_replace(za, file_index, s, ZIP_FL_ENC_UTF_8) < 0) 
  { 
      zip_source_free(s); 
      m_last_error = "can't create archive source";
      return false;
  }

  return true;
}

bool ZipLibWrap::addFile(const std::string& file_name, void* data, int size){
  if(!isValid())
  {
	m_last_error = "archive handle is not valid";
	return false;
  }

  zip_int64_t file_index = zip_name_locate(za,file_name.c_str(),0);
  if (file_index != -1){
	  m_last_error = "file index already exists";
	  return false;
  }

  zip_source_t *s;
//  if ((s=zip_source_buffer(za,data,size,0)) == NULL ||
//		  zip_file_add(za,file_name.c_str(),s,0)){
//      zip_source_free(s);
//      m_last_error = "can't create archive source";
//      return false;
//
//  }

  if ((s=zip_source_buffer(za,data,size,0)) == NULL){
	m_last_error = "can't create archive source";
	return false;
  }

  if (zip_file_add(za,file_name.c_str(),s,0) < 0){
	m_last_error = "can't add file to archive";
	zip_source_free(s);
	return false;
  }

  return true;
};

bool ZipLibWrap::fileExists(const std::string file_name, bool &out_result){
	if(!isValid())
	{
		m_last_error = "archive handle is not valid";
		return false;
	}

	zip_int64_t file_index = zip_name_locate(za,file_name.c_str(),0);
	if (file_index == -1){
		//m_last_error = "file index already exists";
		//return false;
		out_result = false;
	}else{
		out_result = true;
	}
	return true;

};

bool ZipLibWrap::listFiles(std::vector<std::string>& out_res)
{
  if(!isValid())
  {
    m_last_error = "archive handle is not valid";
    return false;
  }

  zip_int64_t  nbEntries = zip_get_num_entries(za, 0);
  
  if(nbEntries == -1)
  {
    m_last_error = "archive is null, no files to list";
    return false;
  }

  for(zip_int64_t  i=0 ; i < nbEntries ; ++i) 
  {
    const char* ret_name = zip_get_name(za, i, 0);

    if(ret_name != NULL)
    {
      out_res.push_back(ret_name);
    }
  }

  return true;
}

bool ZipLibWrap::saveToFile(const std::string& file_name)
{
  std::vector<unsigned char> data;

  if(saveToMem(data))
  {
    if(writeDataToFile(file_name, data))
    {
      return true;
    }    
  }

  return false;
}

bool ZipLibWrap::makeArchFromMem()
{
  zip_error_init(&error);

  /* create source from buffer */
  if ((src = zip_source_buffer_create(m_data, m_data_size, 1, &error)) == NULL) 
  {
    m_last_error = "can't create archive source";
    zip_error_fini(&error);
    return false;
  }

  /* open zip archive from source */
  if ((za = zip_open_from_source(src, 0, &error)) == NULL) 
  {
    m_last_error = "can't open zip from source";
    zip_source_free(src);
    zip_error_fini(&error);
    return false;
  }

  zip_error_fini(&error);

  /* we'll want to read the data back after zip_close */
  zip_source_keep(src);

  return true;
}

bool ZipLibWrap::saveToMem(std::vector<unsigned char>& out_data)
{
  void* buf = NULL;

  /* close archive */
  if (zip_close(za) < 0) {    
    m_last_error = "can't close zip archive handler";
    return false;
  }

  za = NULL;

  /* copy new archive to buffer */
  if (zip_source_is_deleted(src)) 
  {
    /* new archive is empty, thus no data */
    m_last_error = "new archive is empty, thus no data";
    return false;
  }
  else 
  {
    zip_stat_t zst;

    if (zip_source_stat(src, &zst) < 0) {
      m_last_error = "can't stat archive source";
      return false;
    }

    if (zip_source_open(src) < 0) {
      m_last_error = "can't open source";
      return false;
    }
   
    buf = malloc(zst.size);    

    if ((zip_uint64_t)zip_source_read(src, buf, zst.size) < (zip_uint64_t)zst.size) {
        m_last_error = "can't read data from acrh source";
        zip_source_close(src);
        if(buf != NULL) free(buf);

        return false;
    }

    if(zst.size > 0)
    {
      out_data.resize(zst.size);
      memcpy(out_data.data(), buf, zst.size);
    }

    zip_source_close(src);
  }

  /* we're done with src */
  // какая то глюченая либа на каких-то архивах вылетает при очищении на каких-то нет (((
  //zip_source_free(src);
  if(src != NULL) free(src);

  if(buf != NULL) free(buf);
  
  return true;
}

bool ZipLibWrap::loadDataFromFile(const std::string& file_name, std::vector<unsigned char>& data)
{
  struct stat st;
  FILE *fp;  

  if (stat(file_name.c_str(), &st) < 0) 
  {
    m_last_error = "can't stat archive file to read";
    return false;
  }

  data.resize((size_t)st.st_size);

  if ((fp=fopen(file_name.c_str(), "rb")) == NULL) 
  {
    m_last_error = "can't open archive file";
    return false;
  }
    
  if (fread(data.data(), 1, (size_t)st.st_size, fp) < (size_t)st.st_size) 
  {
    m_last_error = "can't read archive file";
    fclose(fp);
    return false;
  }

  fclose(fp);

  return true;
}

bool ZipLibWrap::writeDataToFile(const std::string& file_name, const std::vector<unsigned char>& data)
{
  FILE *fp;

  if (data.size() == 0)
  {
    m_last_error = "no data to write in file";
    return false;
  }

  if ((fp = fopen(file_name.c_str(), "wb")) == NULL) 
  {
    m_last_error = "can't read archive file to write";
    return false;
  }

  if (fwrite(data.data(), 1, data.size(), fp) < (size_t)data.size()) 
  {
    m_last_error = "can't write archive file";
    fclose(fp);
    return false;
  }

  if (fclose(fp) < 0) 
  {
    m_last_error = "can't write archive file";
    return false;
  }

  return true;
}

void ZipLibWrap::freeData()
{
  if(m_data != NULL)
  {
    //delete [] m_data;
    free(m_data);
    m_data = NULL;
    m_data_size = 0;
  }
}

int ZipLibWrap::getDataSize()
{
  return m_data_size;
}

bool ZipLibWrap::isValid()
{
  return za != NULL ? true : false;
}

const std::string& ZipLibWrap::getLastError()
{
  return m_last_error;
}

////////////////////
