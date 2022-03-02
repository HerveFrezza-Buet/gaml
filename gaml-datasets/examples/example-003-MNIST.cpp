#include <iostream>
#include <fstream>
#include <string>

#include <gaml.hpp>
#include <gaml-datasets.hpp>


int main(int argc, char* argv[]) {

  std::random_device rd{};
  std::mt19937 gen{rd()};

  if(argc != 3) {
    std::cout << "Usage : " << argv[0] << " <idx3-ubyte file> <idx1-ubyte file>" << std::endl
	      << "   e.g. " << argv[0] << " train-images-idx3-ubyte train-labels-idx1-ubyte" << std::endl;
    return 0;
  }

  std::string samples_filename = argv[1];
  std::string labels_filename  = argv[2];
  std::string samples_idx_filename = "samples.idx";
  std::string labels_idx_filename  = "labels.idx";

  {
    // This could be done to create the raw MNIST
    // dataset. Nevertheless, we provide an integrated dataset type
    // for that purpose (see after this block of code).
    
    auto sample_parser = gaml::datasets::MNIST::make_input_parser();
    auto label_parser  = gaml::datasets::MNIST::make_label_parser();
    auto raw_samples = gaml::make_indexed_dataset(sample_parser, samples_filename, samples_idx_filename);
    auto raw_labels  = gaml::make_indexed_dataset(label_parser,  labels_filename,  labels_idx_filename );
    auto raw_dataset = gaml::zip(gaml::range(raw_samples.begin(), raw_samples.end()),  
				 gaml::range(raw_labels.begin(),  raw_labels.end() ));
  }

  auto raw_dataset = gaml::datasets::MNIST::dataset({samples_filename, samples_idx_filename},
						    {labels_filename,  labels_idx_filename });

  // We have a {(input1, label1), ... } dataset.

  {
    auto  data_it = raw_dataset.begin() + 3;
    auto& data    = *data_it;  // The fourth data
    gaml::datasets::MNIST::input X = std::get<0>(data);
    gaml::datasets::MNIST::label y = std::get<1>(data);
  }

  // Let us rather cache and shuffle the dataset (this offers
  // efficiency in case of intensive access to the data).
#define CACHE_PAGE_SIZE 10 // ok... not a realistic size (too small).
#define CACHE_NB_PAGES  10

  auto cached  = gaml::cache(raw_dataset.begin(), raw_dataset.end(), CACHE_PAGE_SIZE,CACHE_NB_PAGES);
  auto dataset = gaml::packed_shuffle(cached.begin(), cached.end(), CACHE_PAGE_SIZE, gen);

  

#define NB_DIGITS    30
#define BUF_SIZE     NB_DIGITS*sizeof(gaml::datasets::MNIST::input)
#define IMAGE_WIDTH  NB_DIGITS*28
#define IMAGE_HEIGHT 28
  unsigned char  img_buf[BUF_SIZE];
  unsigned char* pixel = img_buf;
  auto data_it = dataset.begin();
  auto end     = data_it + NB_DIGITS;
  while(data_it != end) {
    auto& [sample, label] = *(data_it++);
    std::cout << (int)label << ' ';
    gaml::datasets::MNIST::draw(pixel, sample, IMAGE_WIDTH);
    pixel += 28;
  }
  std::cout << std::endl;

  std::ofstream ppm_image("digits.ppm");
  ppm_image << "P5\n" << IMAGE_WIDTH << ' ' << IMAGE_HEIGHT << "\n255\n";
  ppm_image.write(reinterpret_cast<char*>(img_buf), BUF_SIZE);
  std::cout << "Image \"digits.ppm\" generated." << std::endl;
  
  return 0;
  
}
