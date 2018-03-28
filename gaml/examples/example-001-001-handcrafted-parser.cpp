#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>

#include <gaml.hpp>

// Let us consider data as tuples u, v, x(u,v), y(u,v), z(u,v).

struct Data {
  double u, v, x, y, z;
};

std::ostream& operator<<(std::ostream& os, const Data& d) {
  os << "(x = " << d.x << ", y = " << d.y << ", z = " << d.z << ')';
  return os;
}


// The values u, v, x, y are stored in a datafile with a CSV format,
// including a 2-line header.
// Let us write a parser for this kind of data.

struct UVXYparser : public gaml::BasicParser {
  using value_type = Data;
  
  void writeBegin(std::ostream& os) const {
    os << "u, v, x(u, v), y(u, v), z(u, v)" << std::endl
       << "-------------------------------" << std::endl;
  }
  
  void writeEnd(std::ostream& os) const {
    // Nope
  }
  
  void writeSeparator(std::ostream& os) const {
    os << std::endl; // The separation between two data.
  }
  
  void readBegin(std::istream& is) const {
    // We skip two lines.
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
  
  void write(std::ostream& os, const Data& data) const {
    os << data.u << ", " << data.v << ", " << data.x << ", " << data.y << ", " << data.z;
  }
  
  void read(std::istream& is, Data& data) const {
    char sep;
    is >> data.u >> sep >> data.v >> sep >> data.x >> sep >> data.y >> sep >> data.z;
  }
};

#define BIG_R    1
#define SMALL_R .2
template<typename RANDOM_DEVICE>
Data sample(RANDOM_DEVICE& rd) {
  std::uniform_real_distribution<double> uniform(0, 2*3.141592654);
  double u = uniform(rd);
  double v = uniform(rd);
  double R = BIG_R + SMALL_R*std::cos(v);
  return { u, v,
      R*std::cos(u),
      R*std::sin(u),
      R*std::sin(v)};
}


#define FILENAME "data.csv"

int main(int argc, char** argv) {
  
  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());

  UVXYparser parser;
  
  {
    // Let us write a data file.
    std::ofstream ofile(FILENAME);
    auto os  = gaml::make_output_data_stream(ofile, parser);
    auto out = gaml::make_output_iterator(os);
    for(unsigned int i=0; i<100; ++i)
      *(out++) = sample(gen);
    std::cout << "File \"" << FILENAME << "\" generated." << std::endl;
  }

  // Now, let us get data from the file.
  std::vector<Data> data;
  {
    std::ifstream ifile(FILENAME);
    auto is    = gaml::make_input_data_stream(ifile, parser);
    auto begin = gaml::make_input_data_begin(is);
    auto end   = gaml::make_input_data_end(is);
    std::copy(begin, end, std::back_inserter(data));
    std::cout << "File \"" << FILENAME << "\" parsed." << std::endl;
  }

  // Let us print some of the data...

  std::cout << data[ 3] << std::endl
	    << data[ 4] << std::endl
	    << data[98] << std::endl;

  return 0;
}
