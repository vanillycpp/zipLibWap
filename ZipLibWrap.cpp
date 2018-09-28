
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ZipLibWrap.h"

ZipLibWrap::ZipLibWrap(const std::string& file_name)
: za(NULL)
, m_data(NULL)
{
  loadDataFromFile(file_name);
  makeArchFromMem();
}

ZipLibWrap::ZipLibWrap(void* data, int data_size)
: za(NULL)
, m_data(data)
, m_data_size(data_size)

{
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
  //const char buf[] = "teststring"; 
   
  if ((s=zip_source_buffer(za, data, size, 0)) == NULL || 
      zip_file_replace(za, file_index, s, ZIP_FL_ENC_UTF_8) < 0) 
  { 
      zip_source_free(s); 
      m_last_error = "can't create archive source";
      return false;
  }

  return true;
}

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
      // char* buf = malloc(strlen(ret_name));
      // strcpy(buf, ret_name);
      // free(buf);
      out_res.push_back(ret_name);
      //printf("%s\n", buf);
    }
  }

  return true;
}

bool ZipLibWrap::saveToMem(void* out_data, int out_data_size)
{
  if(out_data_size < getDataSize())
  {
    m_last_error = "out buffer less then data size";
    return false;
  }

  if(writeArchToMem())
  {
    memcpy(out_data, m_data, m_data_size);
    return true;
  }

  return false;
}

bool ZipLibWrap::saveToFile(const std::string& file_name)
{
  if(writeArchToMem())
  {
    if(writeDataToFile(file_name))
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

bool ZipLibWrap::writeArchToMem()
{
  /* close archive */
  if (zip_close(za) < 0) {    
    m_last_error = "can't close zip archive handler";
    return false;
  }

  za = NULL;

  freeData();

  /* copy new archive to buffer */
  if (zip_source_is_deleted(src)) 
  {
    /* new archive is empty, thus no data */
    freeData();
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

    m_data_size = zst.size;

    if (zip_source_open(src) < 0) {
      m_last_error = "can't open source";
      return false;
    }

    if ((m_data = malloc(m_data_size)) == NULL) {
      m_last_error = "malloc failed";
      zip_source_close(src);
      return false;
    }

    if ((zip_uint64_t)zip_source_read(src, m_data, m_data_size) < (zip_uint64_t)m_data_size) {
        m_last_error = "can't read data from acrh source";
        zip_source_close(src);
        freeData();
        return false;
    }

    zip_source_close(src);
  }

  /* we're done with src */
  zip_source_free(src);

  return true;
}

bool ZipLibWrap::loadDataFromFile(const std::string& file_name)
{
  struct stat st;
  FILE *fp;

  freeData();

  if (stat(file_name.c_str(), &st) < 0) 
  {
    m_last_error = "can't stat archive file to read";
    return false;
  }

  if ((m_data = malloc((size_t)st.st_size)) == NULL) 
  {
    m_last_error = "can't allocate data buffer";
    return false;    
  }

  if ((fp=fopen(file_name.c_str(), "rb")) == NULL) 
  {
    freeData();

    m_last_error = "can't open archive file";
    return false;
  }
    
  if (fread(m_data, 1, (size_t)st.st_size, fp) < (size_t)st.st_size) 
  {
    freeData();

    m_last_error = "can't read archive file";
    fclose(fp);
    return false;
  }

  fclose(fp);

  m_data_size = (size_t)st.st_size;

  return true;
}

bool ZipLibWrap::writeDataToFile(const std::string& file_name)
{
  FILE *fp;

  if (m_data == NULL || m_data_size == 0)
  {
    m_last_error = "no data to write in file";
    return false;
  }

  // if (data == NULL) 
  // {
  //   if (remove(archive) < 0 && errno != ENOENT) {
  //       fprintf(stderr, "can't remove %s: %s\n", archive, strerror(errno));
  //       return -1;
  //   }
  //   return 0;
  // }

  if ((fp = fopen(file_name.c_str(), "wb")) == NULL) 
  {
    m_last_error = "can't read archive file to write";
    return false;
  }

  if (fwrite(m_data, 1, m_data_size, fp) < (size_t)m_data_size) 
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
