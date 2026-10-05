#include "BulletSystem.h"
#include "Collision.h"
#include "UniformGrid.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {
constexpr float kCellSize = 64.0f;
constexpr float kPlayerX = 640.0f;
constexpr float kPlayerY = 360.0f;
constexpr int kIterations = 20000;
constexpr int kRounds = 5;

struct QueryResult {
    std::uint64_t candidates = 0;
    std::uint64_t hits = 0;
    std::uint64_t grazes = 0;
};

void CheckBullet(const Bullet& bullet, QueryResult& result) {
    const CircleHitbox hitbox{kPlayerX, kPlayerY, 5.0f};
    const CircleHitbox grazeBox{kPlayerX, kPlayerY, 24.0f};
    const CircleHitbox bulletBox{bullet.x, bullet.y, 8.0f};
    ++result.candidates;
    const bool hit = Intersects(hitbox, bulletBox);
    result.hits += hit;
    result.grazes += Intersects(grazeBox, bulletBox) && !hit;
}

// Keep each query as a real call during the repeated measurement.
#ifdef _MSC_VER
#define BENCH_NOINLINE __declspec(noinline)
#else
#define BENCH_NOINLINE
#endif

BENCH_NOINLINE QueryResult QueryAll(const std::vector<Bullet>& bullets) {
    QueryResult result;
    for (const auto& bullet : bullets) {
        if (bullet.active && bullet.owner == BulletOwner::Enemy) {
            CheckBullet(bullet, result);
        }
    }
    return result;
}

void Rebuild(UniformGrid& grid, const std::vector<Bullet>& bullets) {
    grid.Clear();
    for (std::size_t i = 0; i < bullets.size(); ++i) {
        const auto& bullet = bullets[i];
        if (!bullet.active || bullet.owner != BulletOwner::Enemy) {
            continue;
        }
        grid.Insert(static_cast<std::size_t>(bullet.x / kCellSize),
                    static_cast<std::size_t>(bullet.y / kCellSize), i);
    }
}

BENCH_NOINLINE QueryResult QueryGrid(const UniformGrid& grid,
                                     const std::vector<Bullet>& bullets) {
    QueryResult result;
    const auto cellX = static_cast<std::size_t>(kPlayerX / kCellSize);
    const auto cellY = static_cast<std::size_t>(kPlayerY / kCellSize);
    const auto minX = cellX > 0 ? cellX - 1 : 0;
    const auto minY = cellY > 0 ? cellY - 1 : 0;
    const auto maxX = std::min(cellX + 1, grid.GetColumns() - 1);
    const auto maxY = std::min(cellY + 1, grid.GetRows() - 1);
    for (auto y = minY; y <= maxY; ++y) {
        for (auto x = minX; x <= maxX; ++x) {
            for (const auto index : grid.GetCell(x, y)) {
                CheckBullet(bullets[index], result);
            }
        }
    }
    return result;
}

volatile std::uint64_t benchmarkSink = 0;

template<class Query>
double Measure(Query query) {
    std::uint64_t checksum = 0;
    const auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < kIterations; ++i) {
        const auto result = query();
        checksum += result.candidates + result.hits + result.grazes;
    }
    const auto end = std::chrono::steady_clock::now();
    benchmarkSink = checksum;
    return std::chrono::duration<double, std::micro>(end - begin).count() /
           kIterations;
}

bool RunCase(std::size_t activeCount) {
    std::vector<Bullet> bullets(256);
    for (std::size_t i = 0; i < activeCount; ++i) {
        auto& bullet = bullets[i];
        bullet.active = true;
        bullet.owner = BulletOwner::Enemy;
        bullet.x = static_cast<float>(25 + (i * 97) % 1230);
        bullet.y = static_cast<float>(25 + (i * 53) % 670);
    }
    // Include one hit and one graze in every case.
    bullets[0].x = kPlayerX;
    bullets[0].y = kPlayerY;
    bullets[1].x = kPlayerX + 20.0f;
    bullets[1].y = kPlayerY;

    UniformGrid grid(20, 12);
    Rebuild(grid, bullets);
    const auto all = QueryAll(bullets);
    const auto nearby = QueryGrid(grid, bullets);
    if (all.hits != nearby.hits || all.grazes != nearby.grazes) {
        std::cerr << "FAIL: Hit/Graze results differ.\n";
        return false;
    }

    const auto bruteQuery = [&] { return QueryAll(bullets); };
    const auto gridQuery = [&] {
        Rebuild(grid, bullets);
        return QueryGrid(grid, bullets);
    };
    // Warm up reusable cell storage before collecting timings.
    (void)Measure(bruteQuery);
    (void)Measure(gridQuery);
    double bruteUs = 0.0;
    double gridUs = 0.0;
    for (int round = 0; round < kRounds; ++round) {
        if (round % 2 == 0) {
            bruteUs += Measure(bruteQuery);
            gridUs += Measure(gridQuery);
        } else {
            gridUs += Measure(gridQuery);
            bruteUs += Measure(bruteQuery);
        }
    }
    std::cout << activeCount << ',' << all.candidates << ','
              << nearby.candidates << ',' << bruteUs / kRounds << ','
              << gridUs / kRounds << ',' << all.hits << ',' << all.grazes
              << '\n';
    return true;
}
} // namespace

int main() {
    std::cout << "CPU collision benchmark; fixed 256-slot pool, one player.\n"
                 "Grid timing includes Clear + Insert + query.\n"
                 "Fixed synthetic positions; no damage or rendering.\n"
                 "Microseconds per update; warmed storage; 5 rounds.\n";
#ifdef _DEBUG
    std::cout << "WARNING: Debug build. Use Release for comparisons.\n";
#endif
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "active,brute_candidates,grid_candidates,brute_us,grid_total_us,hits,grazes\n";
    for (const auto count : {32u, 128u, 256u}) {
        if (!RunCase(count)) {
            return 1;
        }
    }
}
