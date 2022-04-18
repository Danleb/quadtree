#include <light/Quadtree.h>

#include <benchmark/benchmark.h>

#include <random>

constexpr auto POINTS_COUNT = 2 * 1000 * 1000;

void BM_QuadtreePoints(benchmark::State& state)
{
    std::mt19937 rng{};
    std::uniform_int_distribution<int> uid(1, 10000);

    for (auto _ : state)
    {
    }
}

void BM_VectorPoints(benchmark::State& state)
{
    std::mt19937 rng{};
    std::uniform_int_distribution<int> uid(1, 10000);

    for (auto _ : state)
    {
        std::vector<int> v;
        v.reserve(POINTS_COUNT);

        for (size_t i = 0; i < POINTS_COUNT; ++i)
        {
            //
        }
    }
}

BENCHMARK(BM_QuadtreePoints);
BENCHMARK(BM_VectorPoints);
