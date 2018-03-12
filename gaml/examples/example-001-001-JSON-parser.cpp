#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iterator> 

// The ml library.
#include <gaml.hpp>

/*
 * About this example :
 *
 * The following example explains how to read or write datasets by handling them as streams.
 * It shows how to use the JSON built-in parser or to write one's own custom parser.
 * It also shows how to apply STL algorithms to datasets.
 *
 */

// Read this file.
#include <example-customer.hpp>


/*
 * Let us parse Customers.
 */

#define CUSTOMERS_DATA_FILE "customers.data"

int main(int argc, char** argv) {
  try {

    // Let us create some data.

    auto data = customer::make_data();

    // Let us now serialize this data.

    // The gaml library provides a JSON built-in parser. The grammar
    // assumed by the JSON format is the following:
    //   - Each line in the file describes a datum.
    //   - Data fields are separated by comma ',',
    //   - Compound elements (array, vector, list, tuple, pair) are
    //     enclosed within square brackets '[...]'
    //   - Strings are enclosed within double quotes '"..."'
    //   - Maps are enclosed with braces '{...}'. Keys can only be strings.
    // Because the JSON parser is generated specifically for a C++ data type
    // there are some additional requirements.
    //   - Use of STL sequences (vectors, lists and arrays) require that the type
    //     of the elements in the JSON list mapped to the sequence is the same as the value type of the sequence
    //   - STL tuples require that the JSON list mapped to the type contains as many elements as the tuple
    //	   and that their types coincide with the ones of the tuple in the same order.
    //   - STL pairs have the same requirement as tuple since a pair is equivalent to a 2-tuple.
    //   - STL associative containers (maps and hash tables) require that the fields of the JSON object mapped to the container
    //     have all the same types and is compatible with the mapped type of the container.

    auto JSON_parser = gaml::make_JSON_parser<customer::Data>();
    
    {
      // From parsers, output streams can be made, from existing
      // streams. Let us use std::cout here, with the JSON parser.
      auto output_customer_stream_1 = gaml::make_output_data_stream(std::cout, JSON_parser);

      // We use an output iterator to write in the streams.
      auto out1 = gaml::make_output_iterator(output_customer_stream_1);
      std::cout << std::endl << "JSON built-in output parser:\n" << std::endl;
      std::copy(data.begin(), data.end(), out1);
      
      // The destructor of the out1 iterator is called here and
      // outputs the final squared bracket. As we need out1 to be
      // destroyed right now, we have enclosed the current piece of
      // code with {...}
    }

    // Let us serialize this data into a file, using the JSON syntax.
    {
      std::ofstream ofile(CUSTOMERS_DATA_FILE);
      auto output_stream = gaml::make_output_data_stream(ofile, JSON_parser);
      auto out = gaml::make_output_iterator(output_stream);
      std::copy(data.begin(), data.end(), out);
      // out is destroyed before ofile, so the last ']' is written before closing.
    }

    // Now, we can clear the data and re-load it from the file.

    std::cout << std::endl << "Parse JSON file and output content in JSON to the console:\n" << std::endl;
    {
      data.clear();
	
      std::ifstream ifile(CUSTOMERS_DATA_FILE);
      auto input_customer_stream = gaml::make_input_data_stream(ifile, JSON_parser);
      auto output_customer_stream_1 = gaml::make_output_data_stream(std::cout, JSON_parser);
      auto out1 = gaml::make_output_iterator(output_customer_stream_1);

      auto begin = gaml::make_input_data_begin(input_customer_stream);
      auto end = gaml::make_input_data_end(input_customer_stream);
      std::copy(begin, end, std::back_inserter(data));
    }
    
    // Let us display the data again.
    {
      auto output_customer_stream_1 = gaml::make_output_data_stream(std::cout, JSON_parser);
      auto out1 = gaml::make_output_iterator(output_customer_stream_1);
      std::copy(data.begin(), data.end(), out1);
    }
    
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
