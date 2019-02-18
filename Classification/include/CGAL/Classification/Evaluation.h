// Copyright (c) 2017 GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s)     : Simon Giraudot

#ifndef CGAL_CLASSIFICATION_EVALUATION_H
#define CGAL_CLASSIFICATION_EVALUATION_H

#include <CGAL/license/Classification.h>

#include <CGAL/Classification/Label.h>
#include <CGAL/Classification/Label_set.h>
#include <map>
#include <cmath> // for std::isnan

namespace CGAL {

namespace Classification {


/*!
  \ingroup PkgClassificationDataStructures

  \brief Class to compute several measurements to evaluate the quality
  of a classification output.
*/
class Evaluation
{
  const Label_set& m_labels;
  mutable std::map<Label_handle, std::size_t> m_map_labels;
  std::vector<std::vector<std::size_t> > m_confusion; // confusion matrix

public:

  /// \name Constructor
  /// @{

  Evaluation (const Label_set& labels)
    : m_labels (labels)
  {
    init();
  }
  
/*!

  \brief Instantiates an evaluation object and computes all
  measurements.

  \param labels labels used.

  \param ground_truth vector of label indices: it should contain the
  index of the corresponding label in the `Label_set` provided in the
  constructor. Input items that do not have a ground truth information
  should be given the value `-1`.

  \param result similar to `ground_truth` but contained the result of
  a classification.

*/
  template <typename GroundTruthIndexRange, typename ResultIndexRange>
  Evaluation (const Label_set& labels,
              const GroundTruthIndexRange& ground_truth,
              const ResultIndexRange& result)
    : m_labels (labels)
  {
    init();
    append(ground_truth, result);
  }

  /// @}

  /// \cond SKIP_IN_MANUAL
  void init()
  {
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
      m_map_labels[m_labels[i]] = i;

    m_confusion.resize (m_labels.size());
    for (std::size_t i = 0; i < m_confusion.size(); ++ i)
      m_confusion[i].resize (m_labels.size(), 0);
  }

  bool label_has_ground_truth (std::size_t label_idx) const
  {
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
      if (m_confusion[i][label_idx] != 0)
        return true;
    return false;
  }
  /// \endcond

  
  template <typename GroundTruthIndexRange, typename ResultIndexRange>
  void append (const GroundTruthIndexRange& ground_truth,
               const ResultIndexRange& result)
  {
    for (std::size_t i = 0; i < ground_truth.size(); ++ i)
    {
      int gt = static_cast<int>(ground_truth[i]);
      int res = static_cast<int>(result[i]);
      if (gt == -1 || res == -1)
        continue;
      
      ++ m_confusion[std::size_t(res)][std::size_t(gt)];
    }      
  }
  
  /// \name Label Evaluation
  /// @{

  /*!

    \brief Returns the precision of the training for the given label.

    Precision is the number of true positives divided by the sum of
    the true positives and the false positives.

  */
  float precision (Label_handle label) const
  {
    std::size_t idx = m_map_labels[label];
    if (!label_has_ground_truth(idx))
      return std::numeric_limits<float>::quiet_NaN();
    
    std::size_t total = 0;
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
      total += m_confusion[idx][i];

    if (total == 0)
      return 0.f;
    
    return m_confusion[idx][idx] / float(total);
  }

  /*!

    \brief Returns the recall of the training for the given label.

    Recall is the number of true positives divided by the sum of
    the true positives and the false negatives.

  */
  float recall (Label_handle label) const
  {
    std::size_t idx = m_map_labels[label];
    if (!label_has_ground_truth(idx))
      return std::numeric_limits<float>::quiet_NaN();
    
    std::size_t total = 0;
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
      total += m_confusion[i][idx];
    return m_confusion[idx][idx] / float(total);
  }

  /*!

    \brief Returns the \f$F_1\f$ score of the training for the given label.

    \f$F_1\f$ score is the harmonic mean of `precision()` and `recall()`:

    \f[
    F_1 = 2 \times \frac{precision \times recall}{precision + recall}
    \f]

  */
  float f1_score (Label_handle label) const
  {
    float p = precision(label);
    float r = recall(label);

    if (p == 0.f && r == 0.f)
      return 0.f;
    
    return 2.f * p * r / (p + r);
  }

  /*!
    \brief Returns the intersection over union of the training for the
    given label.

    Intersection over union is the number of true positives divided by
    the sum of the true positives, of the false positives and of the
    false negatives.
  */
  float intersection_over_union (Label_handle label) const
  {
    std::size_t idx = m_map_labels[label];
    
    std::size_t total = 0;
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
    {
      total += m_confusion[i][idx];
      if (i != idx)
        total += m_confusion[idx][i];
    }

    return m_confusion[idx][idx] / float(total);
  }

  /// @}
  
  /// \name Global Evaluation
  /// @{

  
  std::size_t number_of_misclassified_items() const
  {
    std::size_t total = 0;
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
      for (std::size_t j = 0; j < m_labels.size(); ++ j)
        if (i != j)
          total += m_confusion[i][j];
    return total;
  }

  std::size_t number_of_items() const
  {
    std::size_t total = 0;
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
      for (std::size_t j = 0; j < m_labels.size(); ++ j)
        total += m_confusion[i][j];
    return total;
  }

  /*!
    \brief Returns the accuracy of the training.

    Accuracy is the total number of true positives divided by the
    total number of provided inliers.
  */
  float accuracy() const
  {
    std::size_t true_positives = 0;
    std::size_t total = 0;
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
    {
      true_positives += m_confusion[i][i];
      for (std::size_t j = 0; j < m_labels.size(); ++ j)
        total += m_confusion[i][j];
    }
    return true_positives / float(total);
  }
  
  /*!
    \brief Returns the mean \f$F_1\f$ score of the training over all
    labels (see `f1_score()`).
  */
  float mean_f1_score() const
  {
    float mean = 0;
    std::size_t nb = 0;
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
      if (label_has_ground_truth(i))
      {
        mean += f1_score(m_labels[i]);
        ++ nb;
      }
    return mean / nb;
  }
  
  /*!
    \brief Returns the mean intersection over union of the training
    over all labels (see `intersection_over_union()`).
  */
  float mean_intersection_over_union() const
  {
    float mean = 0;
    std::size_t nb = 0;
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
    {
      float iou = intersection_over_union(m_labels[i]);
      if (!std::isnan(iou))
      {
        mean += iou;
        ++ nb;
      }
    }
    return mean / nb;
  }

  /// @}

  friend std::ostream& operator<< (std::ostream& os, const Evaluation& evaluation)
  {
    os << "Evaluation of classification:" << std::endl;
    os << " * Global results:" << std::endl;
    os << "   - " << evaluation.number_of_misclassified_items()
       << " misclassified item(s) out of " << evaluation.number_of_items() << std::endl
       << "   - Accuracy = " << evaluation.accuracy() << std::endl
       << "   - Mean F1 score = " << evaluation.mean_f1_score() << std::endl
       << "   - Mean IoU = " << evaluation.mean_intersection_over_union() << std::endl;
    os << " * Detailed results:" << std::endl;
    for (std::size_t i = 0; i < evaluation.m_labels.size(); ++ i)
    {
      os << "   - \"" << evaluation.m_labels[i]->name() << "\": ";
      if (evaluation.label_has_ground_truth(i))
        os << "Precision = " << evaluation.precision(evaluation.m_labels[i]) << " ; "
           << "Recall = " << evaluation.recall(evaluation.m_labels[i]) << " ; "
           << "F1 score = " << evaluation.f1_score(evaluation.m_labels[i]) << " ; "
           << "IoU = " << evaluation.intersection_over_union(evaluation.m_labels[i]) << std::endl;
      else
        os << "(no ground truth)" << std::endl;
    }
    return os;
  }

  static std::ostream& write_evaluation_to_html (std::ostream& os, const Evaluation& evaluation)
  {
    os <<  "<!DOCTYPE html>" << std::endl
       << "<html>" << std::endl
       << "<head>" << std::endl
       << "<style type=\"text/css\">" << std::endl
       << "  body{margin:40px auto; max-width:900px; line-height:1.5; color:#333}" << std::endl
       << "  h1,h2{line-height:1.2}" << std::endl
       << "  table{width:100%}" << std::endl
       << "  table,th,td{border: 1px solid black; border-collapse: collapse; }" << std::endl
       << "  th,td{padding: 5px;}" << std::endl
       << "</style>" << std::endl
       << "<title>Evaluation of CGAL Classification results</title>" << std::endl
       << "</head>" << std::endl
       << "<body>" << std::endl
       << "<h1>Evaluation of CGAL Classification results</h1>" << std::endl;

    os <<  "<h2>Global Results</h2>" << std::endl
       << "<ul>" << std::endl
       << "  <li>" << evaluation.number_of_misclassified_items()
       << " misclassified item(s) out of " << evaluation.number_of_items() << "</li>" << std::endl
       << "  <li>Accuracy = " << evaluation.accuracy() << "</li>" << std::endl
       << "  <li>Mean F1 score = " << evaluation.mean_f1_score() << "</li>" << std::endl
       << "  <li>Mean IoU = " << evaluation.mean_intersection_over_union() << "</li>" << std::endl
       << "</ul>" << std::endl;

    const Label_set& labels = evaluation.m_labels;
    
    os <<  "<h2>Detailed Results</h2>" << std::endl
       << "<table>" << std::endl
       << "  <tr>" << std::endl
       << "    <th><strong>Label</strong></th>" << std::endl
       << "    <th><strong>Precision</strong></th>" << std::endl
       << "    <th><strong>Recall</strong></th>" << std::endl
       << "    <th><strong>F1 score</strong></th>" << std::endl
       << "    <th><strong>IoU</strong></th>" << std::endl
       << "  </tr>" << std::endl;
    for (std::size_t i = 0; i < labels.size(); ++ i)
      if (evaluation.label_has_ground_truth(i))
        os <<  "  <tr>" << std::endl
           << "    <td>" << labels[i]->name() << "</td>" << std::endl
           << "    <td>" << evaluation.precision(labels[i]) << "</td>" << std::endl
           << "    <td>" << evaluation.recall(labels[i]) << "</td>" << std::endl
           << "    <td>" << evaluation.f1_score(labels[i]) << "</td>" << std::endl
           << "    <td>" << evaluation.intersection_over_union(labels[i]) << "</td>" << std::endl
           << "  </tr>" << std::endl;
      else
        os <<  "  <tr>" << std::endl
           << "    <td>" << labels[i]->name() << "</td>" << std::endl
           << "    <td><em>(no ground truth)</em></td>" << std::endl
           << "    <td></td>" << std::endl
           << "    <td></td>" << std::endl
           << "    <td></td>" << std::endl
           << "  </tr>" << std::endl;
        
    os <<  "</table>" << std::endl;

    os <<  "<h2>Confusion Matrix</h2>" << std::endl
       << "<table>" << std::endl
       << "  <tr>" << std::endl
       << "    <th></th>" << std::endl;
    for (std::size_t i = 0; i < labels.size(); ++ i)
      os <<  "    <th><strong>" << labels[i]->name() << "</strong></th>" << std::endl;
    os << "    <th><strong>PREDICTIONS</strong></th>" << std::endl;
    os <<  "  </tr>" << std::endl;

    std::vector<std::size_t> sums (labels.size(), 0);
    for (std::size_t i = 0; i < labels.size(); ++ i)
    {
      os <<  "  <tr>" << std::endl
         << "    <td><strong>" << labels[i]->name() << "</strong></td>" << std::endl;
      std::size_t sum = 0;
      for (std::size_t j = 0; j < labels.size(); ++ j)
      {
        if (i == j)
          os <<  "    <td><strong>" << evaluation.m_confusion[i][j] << "</strong></td>" << std::endl;
        else
          os <<  "    <td>" << evaluation.m_confusion[i][j] << "</td>" << std::endl;
        sum += evaluation.m_confusion[i][j];
        sums[j] += evaluation.m_confusion[i][j];
      }
      os << "    <td><strong>" << sum << "</strong></td>" << std::endl;
      os <<  "  </tr>" << std::endl;
    }
    
    os <<  "  <tr>" << std::endl
       << "    <td><strong>GROUND TRUTH</strong></td>" << std::endl;
    std::size_t total = 0;
    for (std::size_t j = 0; j < labels.size(); ++ j)
    {
      os << "    <td><strong>" << sums[j] << "</strong></td>" << std::endl;
      total += sums[j];
    }
    os << "    <td><strong>" << total << "</strong></td>" << std::endl
       <<  "  </tr>" << std::endl
       <<  "</table>" << std::endl
       << "<p><em>This page was generated by the <a \
href=\"https://doc.cgal.org/latest/Classification/index.html\">CGAL \
Classification package</a>.</em></p>" << std::endl
       <<  "</body>" << std::endl
       << "</html>" << std::endl;
    
    return os;
  }
  
};

  
} // namespace Classification

} // namespace CGAL

#endif // CGAL_CLASSIFICATION_EVALUATION_H

