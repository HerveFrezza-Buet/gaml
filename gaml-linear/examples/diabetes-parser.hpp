
// We create a custom Data type and Parser for diabetes dataset
namespace diabetes { 
  const unsigned int nb_samples = 442;
  const unsigned int nb_features = 10;
  typedef std::array<double, nb_features> X;
  typedef double Y;
  typedef std::pair<X, Y> Data;
  typedef std::vector<Data> Basis;

  void print(const Data& patient) {
    for(unsigned int i = 0 ; i < nb_features; ++i)
      std::cout << patient.first[i] << " ";
    std::cout << " -> " << patient.second << std::endl;
  }

  struct Parser : public gaml::BasicParser {
    using value_type = Data;

    void readBegin(std::istream& is) const {
      // We skip the header
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

    void read(std::istream& is, value_type& patient) const {
      for(unsigned int i = 0 ; i < nb_features; ++i)
	is >> patient.first[i];
      is >> patient.second;
    }


    void writeBegin(std::ostream& os) const {
      // Not implemented
    }
  
    void writeEnd(std::ostream& os) const {
      // Not implemented
    }
  
    void writeSeparator(std::ostream& os) const {
      // Not implemented
    }
    

    void write(std::ostream& os, const value_type& customer) const {
      // Not implemented
    }
  };

  X input_of(const Data& d) {return d.first;}
  Y label_of(const Data& d) {return d.second;}
  void phi(gsl_vector* phi_x, const X& x) {
    for(unsigned int i = 0 ; i < nb_features ; ++i)
      gsl_vector_set(phi_x, i, x[i]);
  }
}
