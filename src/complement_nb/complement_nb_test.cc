#include <complement_nb/complement_nb.h>

#include <fstream>
#include <sstream>

namespace {
bool ParseFile(const char* file_path,
               std::vector<classifier::datum>* data) {
  std::vector<classifier::datum>(0).swap(*data);
  
  std::ifstream ifs(file_path);
  if (!ifs) {
    std::cerr << "cannot open " << file_path << std::endl;
    return false;
  }

  size_t lineN = 0;
  for (std::string line; getline(ifs, line); ++lineN) {
    classifier::datum datum;
    std::istringstream iss(line);

    std::string category = "Not defined";
    if (!(iss >> category)) {
      std::cerr << "parse error: you must set category in line " << lineN << std::endl;
      return false;
    }
    datum.category = category;

    std::vector<std::string> words(0);
    std::string word;
    while (iss >> word) {
      words.push_back(word);
    }

    datum.words = words;
    data->push_back(datum);
  }

  return true;
}

void PrintFeatureScores(const classifier::naivebayes::ComplementNaiveBayes& cnb,
                        const std::vector<classifier::datum>& train) {
  for (size_t i = 0; i < 2; ++i) {
    for (size_t j = 0; j < 3; ++j) {
      std::vector<std::pair<std::string, double> > results(0);
      cnb.CompareFeatureWeight(train[i].words[j], &results);
      std::cout << train[i].words[j] << std::endl;
      for (std::vector<std::pair<std::string, double> >::const_iterator it = results.begin();
           it != results.end();
           ++it) {
        std::cout << it->first << "\t" << it->second << std::endl;
      }
    }
  }
}
} //namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "usage: ./a.out [training file] [test file]" << std::endl;
    return -1;
  }
  
  classifier::naivebayes::ComplementNaiveBayes cnb;
  cnb.set_alpha(1.2);

  std::vector<classifier::datum> train;
  if (!ParseFile(argv[1], &train))
    return -1;
  cnb.Train(train);

  PrintFeatureScores(cnb, train);

  std::vector<classifier::datum> test;
  if (!ParseFile(argv[2], &test))
    return -1;

  size_t score = 0;
  for (size_t i = 0; i < test.size(); ++i) {
    std::string result;
    cnb.Test(test[i], &result);
    if (test[i].category == result)
      ++score;
    std::cout << i << "th data : " << test[i].category << "\t" << result << std::endl;
  }

  std::cout << "accuracy : " << score << " / " << test.size() << std::endl;
  return 0;
}