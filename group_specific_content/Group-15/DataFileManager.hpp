/**
 * @file DataFileManager.hpp
 * @author Zhixiang Miao
 *
 * This link helped me to understand how to trigger functions to build a new row in the file:
 * https://stackoverflow.com/questions/67628186/how-to-write-the-result-of-a-function-to-a-file-c
 */


#pragma once

#include <cassert>
#include <string>
#include <fstream>
#include <iostream>



namespace cse498 {

  class DataFileManager
  {
  private:
    std::string m_filename;
  public:
    DataFileManager(const std::string & filename)
    {
      m_filename = filename;
    }

    int temp1()
    {
      return 2;
    }
    char temp2()
    {
      return 'a';
    }
    std::string temp3()
    {
      return "Hi";
    }

    void update()
    {
      std::ofstream file;
      file.open(m_filename, std::ofstream::app);
      if (!file.is_open())
      {
        std::cerr << "Unable to open file " << m_filename << std::endl;
        return;
      }
      file << temp1() << "," << temp2() << "," << temp3() << std::endl;
      file.close();
    }

  protected:

  };

} // End of namespace cse498