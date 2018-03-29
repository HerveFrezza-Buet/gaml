#pragma once

/*
 *   Copyright (C) 2018,  CentraleSupelec
 *
 *   Author : Jeremy Fix
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : jeremy.fix@centralesupelec.fr
 *
 */

#include <string>
#include <curl/curl.h>
#include <cstdio>
#include <array>
#include <tuple>

#include <experimental/filesystem>

namespace gaml {
  namespace datasets {

    inline std::string download(std::string url) {
      
      auto write_data = [] (void *ptr, size_t size, size_t nmemb, FILE *stream) -> size_t {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
      };
      
      CURL *curl;
      FILE *fp;
      CURLcode res;

      std::string outfilename(std::tmpnam(nullptr));
      while(std::experimental::filesystem::exists(outfilename))
	outfilename = std::tmpnam(nullptr);
     
      
      curl = curl_easy_init();
      if (curl) {
	std::cout << "Downloading " << url << std::endl;
        fp = fopen(outfilename.c_str(),"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	/* enable progress meter */
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        res = curl_easy_perform(curl);
	
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);

	if(res != CURLE_OK)
	  throw std::runtime_error("Failed to download " + url);
      }
      else 
	throw std::runtime_error("Failed to initialize curl");

      return outfilename;
    }

    template<int NB_INPUT,
	     typename OUTPUT>
    class CSVParser : public gaml::BasicParser {
    private:
      unsigned int skiprows;
    public:
      using input_type = std::array<double, NB_INPUT>;
      using output_type = OUTPUT;
      using value_type = std::pair<input_type, output_type>;

      
      CSVParser(unsigned int skiprows): gaml::BasicParser(), skiprows(skiprows) {}
      
      void writeBegin(std::ostream& os) const {
	// Nope
      }
      
      void writeEnd(std::ostream& os) const {
	// Nope
      }
      
      void writeSeparator(std::ostream& os) const {
	os << std::endl; // The separation between two data.
      }
      
      void readBegin(std::istream& is) const {
	// We skip skiprows lines.
	for(unsigned int i = 0 ; i < skiprows; ++i)
	  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
      
      void readEnd(std::istream& is) const {
	// Nope
      }
      
      bool readSeparator(std::istream& is) const {
	// We eat white spaces (as '\n'). 
	is >> std::ws;
	
	// Let us test if more data are available in the file.
	return !is.eof();
      }
      
      void write(std::ostream& os, const value_type& data) const {
	for(const auto& v: data.first)
	  os << v << ", ";
	os << data.second;
      }
      
      void read(std::istream& is, value_type & data) const {
	char sep;
	for(auto& v: data.first)
	  is >> v >> sep;
	is >> data.second;
      }
    };



    
    class Iris {
    public:
      using input_type = std::array<double, 4>;
      using label_type = std::string;
      using data_type = std::pair<input_type, label_type>;
      using dataset_type = std::vector<data_type>;
      
      using output_type = int;
      
    private:
      dataset_type dataset;
      
    public:
           
      Iris() {
	std::string filename = download("https://archive.ics.uci.edu/ml/machine-learning-databases/iris/iris.data");
	
	CSVParser<4, label_type> parser(0);
	
	std::ifstream ifile(filename);
	auto input_data_stream = gaml::make_input_data_stream(ifile, parser);
	auto begin = gaml::make_input_data_begin(input_data_stream);
	auto end = gaml::make_input_data_end(input_data_stream);
	std::copy(begin, end, std::back_inserter(dataset));
      }

      auto begin() {
	return dataset.begin();
      }

      auto end() {
	return dataset.end();
      }

      const input_type& input_of_data(const data_type& data) const {
	return data.first;
      }

      output_type output_of_data(const data_type& data) const {
	if(data.second == std::string("Iris-setosa"))
	  return 0;
	else if(data.second == std::string("Iris-versicolor"))
	  return 1;
	else
	  return 2;
      }
    };
  
    
    struct Diabetes {

    };

    Diabetes diabetes() {
      return Diabetes();
    }
    
    
  }

  gaml::datasets::Iris make_iris_dataset() {
    return gaml::datasets::Iris();
  }

  
}
