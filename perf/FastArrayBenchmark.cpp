#include <light/FastArray.h>

#include <benchmark/benchmark.h>

#include <random>
#include <vector>

constexpr auto ELEMENTS_COUNT = 128;
constexpr auto SWAPS_COUNT = 1000000;

template<template<typename> typename TContainer>
void BM_DataContainerRW(benchmark::State& state)
{
    std::mt19937 rng{};
    std::uniform_int_distribution<int> uid(1, 10000);

    for (auto _ : state)
    {
        TContainer<int> v;
        for (size_t i = 0; i < ELEMENTS_COUNT; ++i)
        {
            v.push_back(uid(rng));
        }

        for (size_t i = 0; i < SWAPS_COUNT; ++i)
        {
            const auto i1 = uid(rng) % ELEMENTS_COUNT;
            const auto i2 = uid(rng) % ELEMENTS_COUNT;

            const auto x = v[i1];
            v[i1] = v[i2];
            v[i2] = x;
        }

        benchmark::DoNotOptimize(v[0]);
    }
}

void BM_FastArrayRW(benchmark::State& state)
{
    BM_DataContainerRW<light::FastArray>(state);
}

void BM_VectorRW(benchmark::State& state)
{
    BM_DataContainerRW<std::vector>(state);
}

BENCHMARK(BM_VectorRW);
BENCHMARK(BM_FastArrayRW);
