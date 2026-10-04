#pragma once

#include <cstddef>
#include <vector>

class UniformGrid {
public:
  UniformGrid(std::size_t columns, std::size_t rows);

  void Resize(std::size_t columns, std::size_t rows);
  void Clear();
  void Insert(std::size_t cellX, std::size_t cellY, std::size_t bulletIndex);

  [[nodiscard]] const std::vector<std::size_t> &
  GetCell(std::size_t cellX, std::size_t cellY) const;

  [[nodiscard]] std::size_t GetColumns() const noexcept { return columns_; }
  [[nodiscard]] std::size_t GetRows() const noexcept { return rows_; }

private:
  [[nodiscard]] std::size_t ToIndex(std::size_t cellX,
                                    std::size_t cellY) const noexcept;

  std::size_t columns_ = 0;
  std::size_t rows_ = 0;
  std::vector<std::vector<std::size_t>> cells_;
};
