#include "UniformGrid.h"

UniformGrid::UniformGrid(std::size_t columns, std::size_t rows)
    : columns_(columns), rows_(rows), cells_(columns * rows) {}

void UniformGrid::Resize(std::size_t columns, std::size_t rows) {
  if (columns_ == columns && rows_ == rows) {
    return;
  }

  columns_ = columns;
  rows_ = rows;
  cells_.clear();
  cells_.resize(columns_ * rows_);
}

void UniformGrid::Clear() {
  for (auto &cell : cells_) {
    cell.clear();
  }
}

void UniformGrid::Insert(std::size_t cellX, std::size_t cellY,
                         std::size_t bulletIndex) {
  cells_[ToIndex(cellX, cellY)].push_back(bulletIndex);
}

const std::vector<std::size_t> &UniformGrid::GetCell(std::size_t cellX,
                                                     std::size_t cellY) const {
  return cells_[ToIndex(cellX, cellY)];
}

std::size_t UniformGrid::ToIndex(std::size_t cellX,
                                 std::size_t cellY) const noexcept {
  return cellY * columns_ + cellX;
}
