/*
 * About this tutorial :
 *
 * Preamble: this tutorial assumes to have already read the tutorial
 * on data parsers and streams
 *
 * The following example explains how to use the indexed dataset
 * class.  An indexed dataset allows to manipulate a potentially very
 * large dataset file as a random access structure, similar to an
 * array.  The class IndexedDataset creates and updates a indexed
 * binary file to index all positions of data in the dataset file.
 * This way it is possible to access randomly any data in the dataset
 * with a practical O(1) time complexity (see note 1 for more
 * explanations).  This feature is particularly interesting when
 * dealing with large datasets that cannot be stored in memory.  The
 * indexed dataset class is conformant with the STL concept of
 * sequential container.  In particular it provides read-only random
 * access iterators to move randomly into file.  Indexed dataset allow
 * to apply cross validation on very large dataset.  When used in
 * conjunction with gaml caching mechanism (gamlCache.hpp),
 *
 * In the following example, one shows how to create an indexed
 * dataset from some dataset file.  One shows how to update the index
 * when new data are appended to the dataset.  Finally one shows how
 * STL algorithms requiring random access iterator can seamlessly be
 * applied to indexed dataset.  Thanks to the indexed file, this
 * operation is done with a spatial complexity of O(1)
 *
 * note 1: because of the way nodes in file systems are organized, a
 * random access to a datum is theoretically of O(log(n)). However the
 * log effect would be noticeable only for huge datasets.  In practice
 * the log effect is not noticeable compared to the time overhead.
 *
 */

// The gaml library.
#include <gaml.hpp>

// Read this file.
#include <example-customer.hpp>

#define CUSTOMERS_DATA_FILE  "customers.data"
#define CUSTOMERS_INDEX_FILE "customers.index"
int main(int argc, char** argv) {
  try {
    
    // This is the parser we need.
    auto parser = gaml::make_JSON_parser<customer::Data>();

    // Let us create a datafile.
    {
      auto data = customer::make_data();
      std::ofstream ofile(CUSTOMERS_DATA_FILE);
      auto output_stream = gaml::make_output_data_stream(ofile, parser);
      {
	auto out           = gaml::make_output_iterator   (output_stream);
	std::copy(data.begin(), data.end(), out);
      }
    }

    // This displays customers on the standard output.
    auto output_stream = gaml::make_output_data_stream(std::cout, parser);

    // Now we can set up an indexed dataset easily...
    auto indexed = gaml::make_indexed_dataset(parser, 
					      CUSTOMERS_DATA_FILE, 
					      CUSTOMERS_INDEX_FILE);

    // ... and have an efficient random access to the elements in the file.
    {
      const auto& customer_3 = indexed[3];
      std::cout << std::endl
		<< "Customer #3" << std::endl
		<< std::endl;
      auto out = gaml::make_output_iterator (output_stream);
      *(out++) = customer_3; // or also : output_stream << customer_3 << std::endl;
    }
    
    // Insertion of new data is also possible (at the end of the file
    // only). For instance let's copy customer_3 at the end of the
    // dataset
    //    indexed.push_back(customer_3);

    // Now, let us append data directly at the end of the file.
    // customer::Article  art1 {"hammer",  8.5};
    // customer::Article  art2 {"saw",    25.3};
    // customer::Article  art3 {"nails",  10.0};
    // customer::Date     day1 {{2014, 12, 23}};
    // customer::Date     day2 {{2014, 12, 26}};
    // customer::Purchase pur1 {day1, {{art1,art3}}};
    // customer::Purchase pur2 {day2, {{art2     }}};
    // 	customer::Ratings  rat1 { { "saw", 1 } };

    // customer::Data     dat1 {false, 43, {{pur1      }}, {}};
    // customer::Data     dat2 {false, 44, {{pur1, pur2}}, rat1};
    
    // std::ofstream ofile(CUSTOMERS_DATA_FILE, std::fstream::app);
    // auto append_stream = gaml::make_output_data_stream(ofile, parser);
    // auto app_out       = gaml::make_output_iterator   (append_stream);
    // // *(app_out++)       = dat1;
    // // *(app_out++)       = dat2;
    // ofile.close();

    // The indexed file "indexed" is not aware of the new data,
    // i.e. it has to update its index table.
    // indexed.update();

    // Let us manipulate the indexed file as any other STL container.

    // We can print it.
    std::cout << std::endl
	      << "All customers" << std::endl
	      << std::endl;
    {
      auto out = gaml::make_output_iterator (output_stream);
      std::copy(indexed.begin(), indexed.end(), out);
    }

    // We can print it in reverse order
    std::cout << std::endl
	      << "All customers (reversed)" << std::endl
	      << std::endl;
    {
      auto out = gaml::make_output_iterator (output_stream);
      std::reverse_copy(indexed.begin(), indexed.end(), out);
    }
  }
  
  catch (const std::exception& e) {
    std::cerr <<"Error: " << e.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
