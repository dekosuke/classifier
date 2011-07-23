#include <passive_aggressive/pa.h>

#include <algorithm>

namespace classifier {
namespace pa {
PA::PA(size_t mode) : mode_(mode), C_(0.001) {
  weight_matrix().swap(weight_);
}

void PA::SetC(double C) {
  if (C > 0.0)
    C_ = C;
}

void PA::Train(const datum& datum) {
  score2class scores(0);
  CalcScores(datum.fv, &scores);
  
  Update(datum.category, scores, datum.fv);
}

void PA::Train(const std::vector<datum>& data,
               const size_t iteration) {
  for (size_t iter = 0; iter < iteration; ++iter) {
    for (size_t i = 0; i < data.size(); ++i) {
      Train(data[i]);
    }
  }
}

void PA::Test(const feature_vector& fv,
              std::string* predict) const {
  score2class scores(0);
  CalcScores(fv, &scores);
  *predict = scores[0].second;
}

void PA::CalcScores(const feature_vector& fv,
                    score2class* scores) const {
  scores->push_back(make_pair(non_class_score, non_class));

  for (weight_matrix::const_iterator it = weight_.begin();
       it != weight_.end();
       ++it) {
    weight_vector wv = it->second;
    double score = InnerProduct(fv, &wv);
    scores->push_back(make_pair(score, it->first));
  }

  sort(scores->begin(), scores->end(),
       std::greater<std::pair<double, std::string> >());
}

void PA::Update(const std::string& correct,
                const score2class& scores,
                const feature_vector& fv) {
  std::string non_correct_predict;
  double hinge_loss = CalcLossScore(scores, correct, &non_correct_predict, 1.0);
  double fv_norm = CalcFvNorm(fv);
  double update = 0.0;

  switch(mode_) {
    case 0:
      update = hinge_loss / fv_norm;
      break;

    case 1:
      update = std::min(hinge_loss / fv_norm, C_);
      break;

    case 2:
      update = hinge_loss / (fv_norm + 1 / (2.0 * C_));
      break;

    default:
      break;
  }
  update /= 2.0;

  if (update > 0.0) {
    for (feature_vector::const_iterator it = fv.begin();
         it != fv.end();
         ++it) {
      weight_[correct][it->first] += update * it->second;
      if (non_correct_predict != non_class)
        weight_[non_correct_predict][it->first] -= update * it->second;
    }
  }
}

void PA::GetFeatureWeight(const std::string& feature,
                          std::vector<std::pair<std::string, double> >* results) const {
  ReturnFeatureWeight(feature, weight_, results);
}

} //namespace
} //namespace
