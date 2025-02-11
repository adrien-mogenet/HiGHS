/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2023 by Julian Hall, Ivet Galabova,    */
/*    Leona Gottwald and Michael Feldmeier                               */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file lp_data/HighsLp.cpp
 * @brief
 */
#include "lp_data/HighsLp.h"

#include <cassert>

#include "util/HighsMatrixUtils.h"

bool HighsLp::isMip() const {
  HighsInt integrality_size = this->integrality_.size();
  if (integrality_size) {
    assert(integrality_size == this->num_col_);
    for (HighsInt iCol = 0; iCol < this->num_col_; iCol++)
      if (this->integrality_[iCol] != HighsVarType::kContinuous) return true;
  }
  return false;
}

bool HighsLp::hasSemiVariables() const {
  HighsInt integrality_size = this->integrality_.size();
  if (integrality_size) {
    assert(integrality_size == this->num_col_);
    for (HighsInt iCol = 0; iCol < this->num_col_; iCol++)
      if (this->integrality_[iCol] == HighsVarType::kSemiContinuous ||
          this->integrality_[iCol] == HighsVarType::kSemiInteger)
        return true;
  }
  return false;
}

bool HighsLp::operator==(const HighsLp& lp) const {
  bool equal = equalButForNames(lp);
  equal = equalNames(lp) && equal;
  return equal;
}

bool HighsLp::equalNames(const HighsLp& lp) const {
  bool equal = true;
  equal = this->objective_name_ == lp.objective_name_ && equal;
  equal = this->row_names_ == lp.row_names_ && equal;
  equal = this->col_names_ == lp.col_names_ && equal;
  return equal;
}

bool HighsLp::equalButForNames(const HighsLp& lp) const {
  bool equal = true;
  equal = this->num_col_ == lp.num_col_ && equal;
  equal = this->num_row_ == lp.num_row_ && equal;
  equal = this->sense_ == lp.sense_ && equal;
  equal = this->offset_ == lp.offset_ && equal;
  equal = this->model_name_ == lp.model_name_ && equal;
  equal = this->col_cost_ == lp.col_cost_ && equal;
  equal = this->col_upper_ == lp.col_upper_ && equal;
  equal = this->col_lower_ == lp.col_lower_ && equal;
  equal = this->row_upper_ == lp.row_upper_ && equal;
  equal = this->row_lower_ == lp.row_lower_ && equal;

  equal = this->a_matrix_ == lp.a_matrix_;

  equal = this->scale_.strategy == lp.scale_.strategy && equal;
  equal = this->scale_.has_scaling == lp.scale_.has_scaling && equal;
  equal = this->scale_.num_col == lp.scale_.num_col && equal;
  equal = this->scale_.num_row == lp.scale_.num_row && equal;
  equal = this->scale_.cost == lp.scale_.cost && equal;
  equal = this->scale_.col == lp.scale_.col && equal;
  equal = this->scale_.row == lp.scale_.row && equal;
  return equal;
}

double HighsLp::objectiveValue(const std::vector<double>& solution) const {
  assert((int)solution.size() >= this->num_col_);
  double objective_function_value = this->offset_;
  for (HighsInt iCol = 0; iCol < this->num_col_; iCol++)
    objective_function_value += this->col_cost_[iCol] * solution[iCol];
  return objective_function_value;
}

HighsCDouble HighsLp::objectiveCDoubleValue(
    const std::vector<double>& solution) const {
  assert((int)solution.size() >= this->num_col_);
  HighsCDouble objective_function_value = this->offset_;
  for (HighsInt iCol = 0; iCol < this->num_col_; iCol++)
    objective_function_value += this->col_cost_[iCol] * solution[iCol];
  return objective_function_value;
}

void HighsLp::setMatrixDimensions() {
  this->a_matrix_.num_col_ = this->num_col_;
  this->a_matrix_.num_row_ = this->num_row_;
}

void HighsLp::resetScale() {
  // Should allow user-supplied scale to be retained
  //  const bool dimensions_ok =
  //    this->scale_.num_col_ == this->num_col_ &&
  //    this->scale_.num_row_ == this->num_row_;
  this->clearScale();
}

void HighsLp::setFormat(const MatrixFormat format) {
  this->a_matrix_.setFormat(format);
}

void HighsLp::exactResize() {
  this->col_cost_.resize(this->num_col_);
  this->col_lower_.resize(this->num_col_);
  this->col_upper_.resize(this->num_col_);
  this->row_lower_.resize(this->num_row_);
  this->row_upper_.resize(this->num_row_);
  this->a_matrix_.exactResize();

  if ((int)this->col_names_.size()) this->col_names_.resize(this->num_col_);
  if ((int)this->row_names_.size()) this->row_names_.resize(this->num_row_);
  if ((int)this->integrality_.size()) this->integrality_.resize(this->num_col_);
}

void HighsLp::clear() {
  this->num_col_ = 0;
  this->num_row_ = 0;

  this->col_cost_.clear();
  this->col_lower_.clear();
  this->col_upper_.clear();
  this->row_lower_.clear();
  this->row_upper_.clear();

  this->a_matrix_.clear();

  this->sense_ = ObjSense::kMinimize;
  this->offset_ = 0;

  this->model_name_ = "";
  this->objective_name_ = "";

  this->new_col_name_ix_ = 0;
  this->new_row_name_ix_ = 0;
  this->col_names_.clear();
  this->row_names_.clear();

  this->integrality_.clear();

  this->col_hash_.clear();
  this->row_hash_.clear();

  this->clearScale();
  this->is_scaled_ = false;
  this->is_moved_ = false;
  this->cost_row_location_ = -1;
  this->mods_.clear();
}

void HighsLp::clearScale() {
  this->scale_.strategy = kSimplexScaleStrategyOff;
  this->scale_.has_scaling = false;
  this->scale_.num_col = 0;
  this->scale_.num_row = 0;
  this->scale_.cost = 0;
  this->scale_.col.clear();
  this->scale_.row.clear();
}

void HighsLp::clearScaling() {
  this->unapplyScale();
  this->clearScale();
}

void HighsLp::applyScale() {
  // Ensure that any scaling is applied
  const HighsScale& scale = this->scale_;
  if (this->is_scaled_) {
    // Already scaled - so check that there is scaling and return
    assert(scale.has_scaling);
    return;
  }
  // No scaling currently applied
  this->is_scaled_ = false;
  if (scale.has_scaling) {
    // Apply the scaling to the bounds, costs and matrix, and record
    // that it has been applied
    for (HighsInt iCol = 0; iCol < this->num_col_; iCol++) {
      this->col_lower_[iCol] /= scale.col[iCol];
      this->col_upper_[iCol] /= scale.col[iCol];
      this->col_cost_[iCol] *= scale.col[iCol];
    }
    for (HighsInt iRow = 0; iRow < this->num_row_; iRow++) {
      this->row_lower_[iRow] *= scale.row[iRow];
      this->row_upper_[iRow] *= scale.row[iRow];
    }
    this->a_matrix_.applyScale(scale);
    this->is_scaled_ = true;
  }
}

void HighsLp::unapplyScale() {
  // Ensure that any scaling is not applied
  const HighsScale& scale = this->scale_;
  if (!this->is_scaled_) {
    // Not scaled so return
    return;
  }
  // Already scaled - so check that there is scaling
  assert(scale.has_scaling);
  // Unapply the scaling to the bounds, costs and matrix, and record
  // that it has been unapplied
  for (HighsInt iCol = 0; iCol < this->num_col_; iCol++) {
    this->col_lower_[iCol] *= scale.col[iCol];
    this->col_upper_[iCol] *= scale.col[iCol];
    this->col_cost_[iCol] /= scale.col[iCol];
  }
  for (HighsInt iRow = 0; iRow < this->num_row_; iRow++) {
    this->row_lower_[iRow] /= scale.row[iRow];
    this->row_upper_[iRow] /= scale.row[iRow];
  }
  this->a_matrix_.unapplyScale(scale);
  this->is_scaled_ = false;
}

void HighsLp::moveBackLpAndUnapplyScaling(HighsLp& lp) {
  assert(this->is_moved_ == true);
  *this = std::move(lp);
  this->unapplyScale();
  assert(this->is_moved_ == false);
}

void HighsLp::addColNames(const std::string name, const HighsInt num_new_col) {
  // Don't add names if there are no columns, or if the names are
  // already incomplete
  if (this->num_col_ == 0) return;
  HighsInt col_names_size = this->col_names_.size();
  if (col_names_size < this->num_col_) return;
  if (!this->col_hash_.name2index.size())
    this->col_hash_.form(this->col_names_);
  // Handle the addition of user-defined names later
  assert(name == "");
  for (HighsInt iCol = this->num_col_; iCol < this->num_col_ + num_new_col;
       iCol++) {
    const std::string col_name =
        "col_ekk_" + std::to_string(this->new_col_name_ix_++);
    bool added = false;
    auto search = this->col_hash_.name2index.find(col_name);
    if (search == this->col_hash_.name2index.end()) {
      // Name not found in hash
      if (col_names_size == this->num_col_) {
        // No space (or name) for this col name
        this->col_names_.push_back(col_name);
        added = true;
      } else if (col_names_size > iCol) {
        // Space for this col name. Only add if name is blank
        if (this->col_names_[iCol] == "") {
          this->col_names_[iCol] = col_name;
          added = true;
        }
      }
    }
    if (added) {
      const bool duplicate =
          !this->col_hash_.name2index.emplace(col_name, iCol).second;
      assert(!duplicate);
      assert(this->col_names_[iCol] == col_name);
      assert(this->col_hash_.name2index.find(col_name)->second == iCol);
    } else {
      // Duplicate name or other failure
      this->col_hash_.name2index.clear();
      return;
    }
  }
}

void HighsLp::addRowNames(const std::string name, const HighsInt num_new_row) {
  // Don't add names if there are no rows, or if the names are already
  // incomplete
  if (this->num_row_ == 0) return;
  HighsInt row_names_size = this->row_names_.size();
  if (row_names_size < this->num_row_) return;
  if (!this->row_hash_.name2index.size())
    this->row_hash_.form(this->row_names_);
  // Handle the addition of user-defined names later
  assert(name == "");
  for (HighsInt iRow = this->num_row_; iRow < this->num_row_ + num_new_row;
       iRow++) {
    const std::string row_name =
        "row_ekk_" + std::to_string(this->new_row_name_ix_++);
    bool added = false;
    auto search = this->row_hash_.name2index.find(row_name);
    if (search == this->row_hash_.name2index.end()) {
      // Name not found in hash
      if (row_names_size == this->num_row_) {
        // No space (or name) for this row name
        this->row_names_.push_back(row_name);
        added = true;
      } else if (row_names_size > iRow) {
        // Space for this row name. Only add if name is blank
        if (this->row_names_[iRow] == "") {
          this->row_names_[iRow] = row_name;
          added = true;
        }
      }
    }
    if (added) {
      const bool duplicate =
          !this->row_hash_.name2index.emplace(row_name, iRow).second;
      assert(!duplicate);
      assert(this->row_names_[iRow] == row_name);
      assert(this->row_hash_.name2index.find(row_name)->second == iRow);
    } else {
      // Duplicate name or other failure
      this->row_hash_.name2index.clear();
      return;
    }
  }
}

void HighsLp::unapplyMods() {
  // Restore any non-semi types
  const HighsInt num_non_semi = this->mods_.save_non_semi_variable_index.size();
  for (HighsInt k = 0; k < num_non_semi; k++) {
    HighsInt iCol = this->mods_.save_non_semi_variable_index[k];
    assert(this->integrality_[iCol] == HighsVarType::kContinuous ||
           this->integrality_[iCol] == HighsVarType::kInteger);
    if (this->integrality_[iCol] == HighsVarType::kContinuous) {
      this->integrality_[iCol] = HighsVarType::kSemiContinuous;
    } else {
      this->integrality_[iCol] = HighsVarType::kSemiInteger;
    }
  }
  // Restore any inconsistent semi variables
  const HighsInt num_inconsistent_semi =
      this->mods_.save_inconsistent_semi_variable_index.size();
  if (!num_inconsistent_semi) {
    assert(
        !this->mods_.save_inconsistent_semi_variable_lower_bound_value.size());
    assert(
        !this->mods_.save_inconsistent_semi_variable_upper_bound_value.size());
    assert(!this->mods_.save_inconsistent_semi_variable_type.size());
  }
  for (HighsInt k = 0; k < num_inconsistent_semi; k++) {
    HighsInt iCol = this->mods_.save_inconsistent_semi_variable_index[k];
    this->col_lower_[iCol] =
        this->mods_.save_inconsistent_semi_variable_lower_bound_value[k];
    this->col_upper_[iCol] =
        this->mods_.save_inconsistent_semi_variable_upper_bound_value[k];
    this->integrality_[iCol] =
        this->mods_.save_inconsistent_semi_variable_type[k];
  }
  // Restore any relaxed lower bounds
  std::vector<HighsInt>& relaxed_semi_variable_lower_index =
      this->mods_.save_relaxed_semi_variable_lower_bound_index;
  std::vector<double>& relaxed_semi_variable_lower_value =
      this->mods_.save_relaxed_semi_variable_lower_bound_value;
  const HighsInt num_lower_bound = relaxed_semi_variable_lower_index.size();
  if (!num_lower_bound) {
    assert(!relaxed_semi_variable_lower_value.size());
  }
  for (HighsInt k = 0; k < num_lower_bound; k++) {
    HighsInt iCol = relaxed_semi_variable_lower_index[k];
    assert(this->integrality_[iCol] == HighsVarType::kSemiContinuous ||
           this->integrality_[iCol] == HighsVarType::kSemiInteger);
    this->col_lower_[iCol] = relaxed_semi_variable_lower_value[k];
  }
  // Restore any tightened upper bounds
  std::vector<HighsInt>& tightened_semi_variable_upper_bound_index =
      this->mods_.save_tightened_semi_variable_upper_bound_index;
  std::vector<double>& tightened_semi_variable_upper_bound_value =
      this->mods_.save_tightened_semi_variable_upper_bound_value;
  const HighsInt num_upper_bound =
      tightened_semi_variable_upper_bound_index.size();
  if (!num_upper_bound) {
    assert(!tightened_semi_variable_upper_bound_value.size());
  }
  for (HighsInt k = 0; k < num_upper_bound; k++) {
    HighsInt iCol = tightened_semi_variable_upper_bound_index[k];
    assert(this->integrality_[iCol] == HighsVarType::kSemiContinuous ||
           this->integrality_[iCol] == HighsVarType::kSemiInteger);
    this->col_upper_[iCol] = tightened_semi_variable_upper_bound_value[k];
  }

  this->mods_.clear();
}

void HighsLpMods::clear() {
  this->save_non_semi_variable_index.clear();
  this->save_inconsistent_semi_variable_index.clear();
  this->save_inconsistent_semi_variable_lower_bound_value.clear();
  this->save_inconsistent_semi_variable_upper_bound_value.clear();
  this->save_inconsistent_semi_variable_type.clear();
  this->save_relaxed_semi_variable_lower_bound_index.clear();
  this->save_relaxed_semi_variable_lower_bound_value.clear();
  this->save_tightened_semi_variable_upper_bound_index.clear();
  this->save_tightened_semi_variable_upper_bound_value.clear();
}

bool HighsLpMods::isClear() {
  if (this->save_non_semi_variable_index.size()) return false;
  if (this->save_inconsistent_semi_variable_index.size()) return false;
  if (this->save_inconsistent_semi_variable_lower_bound_value.size())
    return false;
  if (this->save_inconsistent_semi_variable_upper_bound_value.size())
    return false;
  if (this->save_inconsistent_semi_variable_type.size()) return false;
  if (this->save_relaxed_semi_variable_lower_bound_value.size()) return false;
  if (this->save_relaxed_semi_variable_lower_bound_value.size()) return false;
  if (this->save_tightened_semi_variable_upper_bound_index.size()) return false;
  if (this->save_tightened_semi_variable_upper_bound_value.size()) return false;
  return true;
}

void HighsNameHash::form(const std::vector<std::string>& name) {
  HighsInt num_name = name.size();
  this->clear();
  for (HighsInt index = 0; index < num_name; index++) {
    const bool duplicate = !this->name2index.emplace(name[index], index).second;
    if (duplicate) {
      // Find the original and mark it as duplicate
      auto search = this->name2index.find(name[index]);
      assert(search != this->name2index.end());
      assert(int(search->second) < int(this->name2index.size()));
      this->name2index.erase(search);
      this->name2index.insert({name[index], kHashIsDuplicate});
    }
  }
}

bool HighsNameHash::hasDuplicate(const std::vector<std::string>& name) {
  HighsInt num_name = name.size();
  this->clear();
  bool has_duplicate = false;
  for (HighsInt index = 0; index < num_name; index++) {
    has_duplicate = !this->name2index.emplace(name[index], index).second;
    if (has_duplicate) break;
  }
  this->clear();
  return has_duplicate;
}

void HighsNameHash::clear() { this->name2index.clear(); }
