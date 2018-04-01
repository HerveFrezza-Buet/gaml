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
	std::cout << std::endl;
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

    template<typename INPUT,
	     typename OUTPUT,
	     typename PARSE_FIELD>
    class CSVParser : public gaml::BasicParser {
    private:
      char expected_sep;
      unsigned int skiprows;
      unsigned int nb_fields;
      PARSE_FIELD parse_field;
    public:
      using input_type = INPUT;
      using output_type = OUTPUT;
      using value_type = std::pair<input_type, output_type>;

      
      CSVParser(char expected_sep,
		unsigned int skiprows, unsigned int nb_fields,
		const PARSE_FIELD& parse_field):
	gaml::BasicParser(),
	expected_sep(expected_sep),
	skiprows(skiprows), nb_fields(nb_fields),
	parse_field(parse_field) {}
      
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
	for(unsigned int i = 0 ; i < nb_fields; ++i) {
	  parse_field(is, i, data);
	  if(expected_sep != ' ' && ((i+1) != nb_fields)) { 
	    is >> sep;
	    if(sep != expected_sep) 
	      throw std::runtime_error("I was expecting the separator '" + std::string(1, expected_sep) + "' but got '" + std::string(1, sep) + "'");
	  }
	}
      }
    };

    template<typename INPUT,
	     typename OUTPUT,
	     typename PARSE_FIELD>
    CSVParser<INPUT, OUTPUT, PARSE_FIELD> make_csv_parser(char expected_sep,
							  unsigned int skip_rows,
							  unsigned int nb_fields,
							  const PARSE_FIELD& parse_field) {
      return CSVParser<INPUT, OUTPUT, PARSE_FIELD>(expected_sep, skip_rows, nb_fields, parse_field);
    }
    
    
    template<typename PARSER>
    class DownloadedDataset {
    public:
      using input_type = typename PARSER::input_type;
      using output_type = typename PARSER::output_type;
      using data_type = std::pair<input_type, output_type>;
      using dataset_type = std::vector<data_type>;
      
    private:
      dataset_type dataset;

    public:

      DownloadedDataset(std::string url,
		      const PARSER& parser) {
	std::string filename = download(url);
	
	std::ifstream ifile(filename);
	auto input_data_stream = gaml::make_input_data_stream(ifile, parser);
	auto begin = gaml::make_input_data_begin(input_data_stream);
	auto end = gaml::make_input_data_end(input_data_stream);
	std::copy(begin, end, std::back_inserter(dataset));
	ifile.close();
      }
      
      auto begin() const {
	return dataset.begin();
      }

      auto end() const {
	return dataset.end();
      }

      const input_type& input_of_data(const data_type& data) const {
	return data.first;
      }

      const output_type& output_of_data(const data_type& data) const {
	return data.second;
      }      
    };

    template<typename PARSER>
    DownloadedDataset<PARSER> make_downloaded_dataset(std::string url,
						      const PARSER& parser) {
      return DownloadedDataset<PARSER>(url, parser);
    }
    
  }

  inline auto make_iris_dataset() {
    std::string url("https://archive.ics.uci.edu/ml/machine-learning-databases/iris/iris.data");
    // Fields : a b c d label
    using input_type = std::array<double, 4>;
    using output_type = int;
    auto parse_field = [](std::istream& is, unsigned int field_idx, std::pair<input_type, output_type>& data) {
      if(field_idx < 4) 
	is >> data.first[field_idx];
      else if(field_idx == 4) {
	std::string label;
	is >> label;
	if(label == std::string("Iris-setosa"))
	  data.second = 0;
	else if(label == std::string("Iris-versicolor"))
	  data.second = 1;
	else
	  data.second = 2;
      }
    };
    
    auto parser = gaml::datasets::make_csv_parser<input_type, output_type>(',', 0, 5, parse_field);
    return gaml::datasets::make_downloaded_dataset(url, parser);
  }

  inline auto make_diabetes_dataset() {
    std::string url("https://www4.stat.ncsu.edu/~boos/var.select/diabetes.tab.txt");
    using input_type = std::array<double, 10>;
    using output_type = int;
    auto parse_field = [](std::istream& is, unsigned int field_idx, std::pair<input_type, output_type>& data) {
      if(field_idx < 10) 
	is >> data.first[field_idx];
      else
	is >> data.second;
    };
    
    auto parser = gaml::datasets::make_csv_parser<input_type, output_type>(' ', 1, 11, parse_field);
    return gaml::datasets::make_downloaded_dataset(url, parser);
  }

  /*
    In this dataset, the class is the first element
    our parser cannot handle this yet
    
    inline auto make_wine_dataset() {
    std::string url("https://archive.ics.uci.edu/ml/machine-learning-databases/wine/wine.data");
    auto output_of_label = [](int label) {
    return label;
    };
    
    auto parser = gaml::datasets::make_csv_parser<10, int>(' ', 0, output_of_label);
    return gaml::datasets::make_downloaded_dataset(url, parser);
    }
  */
  
 inline auto make_boston_housing_dataset() {
   std::string url("https://archive.ics.uci.edu/ml/machine-learning-databases/housing/housing.data");
   using input_type = std::array<double, 13>;
   using output_type = double;
   auto parse_field = [](std::istream& is, unsigned int field_idx, std::pair<input_type, output_type>& data) {
     if(field_idx < 13) 
       is >> data.first[field_idx];
     else
       is >> data.second;
   };
   
   auto parser = gaml::datasets::make_csv_parser<input_type, output_type>(' ', 0, 14, parse_field); 
   return gaml::datasets::make_downloaded_dataset(url, parser);
 }
  
}
