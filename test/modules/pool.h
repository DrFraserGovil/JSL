#include "../test_utils/catch_extended.h"
#include "catch2/matchers/catch_matchers_string.hpp"
#include <JSL/Async/ParallelPool.h>
#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <numeric>
#include <vector>

using namespace JSL::Async;
using namespace std::chrono_literals;

// -----------------------------------------------------------------------
// Construction
// -----------------------------------------------------------------------

TEST_CASE("Pool constructs and destructs cleanly with a normal core count", "[Pool][construction]")
{
	REQUIRE_NOTHROW([] { Pool pool(4); }());
}

TEST_CASE("Pool falls back to 1 worker when constructed with 0 cores", "[Pool][construction]")
{
	// We can't easily assert on the LOG(WARN) output, but we *can* assert
	// the pool is still usable afterwards (i.e. it didn't spin up 0 threads
	// and silently deadlock every task).
	auto txt = capture_stdout([&]() {
		Pool pool(0);
		std::atomic<bool> ran = false;
		pool.AsyncTask([&] { ran = true; });
		pool.Synchronise();
		REQUIRE(ran.load());
	});

	REQUIRE_THAT(txt, Catch::Matchers::ContainsSubstring("WARN"));
}

// -----------------------------------------------------------------------
// AsyncTask
// -----------------------------------------------------------------------

TEST_CASE("AsyncTask executes a single submitted task", "[Pool][AsyncTask]")
{
	Pool pool(2);
	std::atomic<int> counter = 0;
	pool.AsyncTask([&] { ++counter; });
	pool.Synchronise();
	REQUIRE(counter.load() == 1);
}

TEST_CASE("AsyncTask executes every submitted task exactly once", "[Pool][AsyncTask]")
{
	Pool pool(4);
	constexpr int N = 500;
	std::vector<std::atomic<int>> hits(N);
	for (auto &h : hits) h = 0;

	for (int i = 0; i < N; ++i)
	{
		pool.AsyncTask([&hits, i] { ++hits[i]; });
	}
	pool.Synchronise();

	for (int i = 0; i < N; ++i)
	{
		REQUIRE(hits[i].load() == 1);
	}
}

TEST_CASE("Synchronise blocks until all outstanding tasks have completed", "[Pool][Synchronise]")
{
	Pool pool(4);
	std::atomic<int> completed = 0;
	constexpr int N = 50;

	for (int i = 0; i < N; ++i)
	{
		pool.AsyncTask([&] {
			std::this_thread::sleep_for(2ms);
			++completed;
		});
	}
	pool.Synchronise();
	// If Synchronise() returned early, this would very likely fail.
	REQUIRE(completed.load() == N);
}

TEST_CASE("Pool can be reused for multiple rounds of tasks after Synchronise", "[Pool][Synchronise]")
{
	Pool pool(3);
	std::atomic<int> counter = 0;

	for (int round = 0; round < 5; ++round)
	{
		for (int i = 0; i < 10; ++i)
		{
			pool.AsyncTask([&] { ++counter; });
		}
		pool.Synchronise();
	}
	REQUIRE(counter.load() == 50);
}

// -----------------------------------------------------------------------
// AsyncReturn
// -----------------------------------------------------------------------

TEST_CASE("AsyncReturn produces a future with the correct value", "[Pool][AsyncReturn]")
{
	Pool pool(2);
	std::future<int> f = pool.AsyncReturn<int>([] { return 42; });
	pool.Synchronise();
	REQUIRE(f.get() == 42);
}

TEST_CASE("AsyncReturn futures are all ready after Synchronise", "[Pool][AsyncReturn]")
{
	Pool pool(4);
	std::vector<std::future<int>> futures;
	for (int i = 0; i < 20; ++i)
	{
		futures.push_back(pool.AsyncReturn<int>([i] { return i * i; }));
	}
	pool.Synchronise();

	for (int i = 0; i < 20; ++i)
	{
		using namespace std::chrono_literals;
		// Should already be ready -- this should never actually wait.
		REQUIRE(futures[i].wait_for(0s) == std::future_status::ready);
		REQUIRE(futures[i].get() == i * i);
	}
}

TEST_CASE("AsyncReturn captures an exception in the future rather than crashing the pool", "[Pool][AsyncReturn][exceptions]")
{
	Pool pool(2);
	std::future<int> f = pool.AsyncReturn<int>([]() -> int {
		throw std::runtime_error("AsyncReturn failure");
	});

	// Synchronise() should NOT throw for AsyncReturn failures -- the
	// exception belongs to the future, not the pool.
	REQUIRE_NOTHROW(pool.Synchronise());
	REQUIRE_THROWS_AS(f.get(), std::runtime_error);
}

// -----------------------------------------------------------------------
// Exception propagation through Synchronise (plain AsyncTask / LoopTask)
// -----------------------------------------------------------------------

TEST_CASE("Synchronise rethrows an exception from a plain AsyncTask", "[Pool][exceptions]")
{
	Pool pool(2);
	pool.AsyncTask([] { throw std::runtime_error("boom"); });
	REQUIRE_THROWS_AS(pool.Synchronise(), std::runtime_error);
}

TEST_CASE("Synchronise does not rethrow a stale exception on a later call", "[Pool][exceptions]")
{
	Pool pool(2);
	pool.AsyncTask([] { throw std::runtime_error("first failure"); });
	REQUIRE_THROWS_AS(pool.Synchronise(), std::runtime_error);

	// A clean batch of work afterwards should not resurrect the old error.
	std::atomic<bool> ran = false;
	pool.AsyncTask([&] { ran = true; });
	REQUIRE_NOTHROW(pool.Synchronise());
	REQUIRE(ran.load());
}

TEST_CASE("Pool keeps running other tasks in a batch after one task throws", "[Pool][exceptions]")
{
	Pool pool(4);
	std::atomic<int> completed = 0;
	constexpr int N = 20;

	for (int i = 0; i < N; ++i)
	{
		pool.AsyncTask([&, i] {
			if (i == N / 2)
			{
				throw std::runtime_error("one bad task");
			}
			++completed;
		});
	}
	REQUIRE_THROWS_AS(pool.Synchronise(), std::runtime_error);
	// Every *other* task should still have run to completion.
	REQUIRE(completed.load() == N - 1);
}

// -----------------------------------------------------------------------
// LoopTask distribution policies
// -----------------------------------------------------------------------

TEST_CASE("LoopTask (Sequential) visits every index exactly once", "[Pool][LoopTask]")
{
	Pool pool(4);
	constexpr size_t N = 1000;
	std::vector<std::atomic<int>> hits(N);
	for (auto &h : hits) h = 0;

	pool.LoopTask(N, [&](size_t i) { ++hits[i]; }, DistributionPolicy::Sequential);
	pool.Synchronise();

	for (size_t i = 0; i < N; ++i)
	{
		REQUIRE(hits[i].load() == 1);
	}
}

TEST_CASE("LoopTask (Balanced) visits every index exactly once", "[Pool][LoopTask]")
{
	Pool pool(4);
	constexpr size_t N = 1000;
	std::vector<std::atomic<int>> hits(N);
	for (auto &h : hits) h = 0;

	pool.LoopTask(N, [&](size_t i) { ++hits[i]; }, DistributionPolicy::Balanced);
	pool.Synchronise();

	for (size_t i = 0; i < N; ++i)
	{
		REQUIRE(hits[i].load() == 1);
	}
}

TEST_CASE("LoopTask handles nLoop == 0 without hanging", "[Pool][LoopTask][edge-case]")
{
	Pool pool(4);
	REQUIRE_NOTHROW(pool.LoopTask(0, [](size_t) { FAIL("func should never be invoked for nLoop == 0"); }));
	REQUIRE_NOTHROW(pool.Synchronise());
}

TEST_CASE("LoopTask handles nLoop smaller than worker count", "[Pool][LoopTask][edge-case]")
{
	Pool pool(8);
	constexpr size_t N = 3;
	std::vector<std::atomic<int>> hits(N);
	for (auto &h : hits) h = 0;

	pool.LoopTask(N, [&](size_t i) { ++hits[i]; });
	pool.Synchronise();

	for (size_t i = 0; i < N; ++i)
	{
		REQUIRE(hits[i].load() == 1);
	}
}

// -----------------------------------------------------------------------
// LoopReturn
// -----------------------------------------------------------------------

TEST_CASE("LoopReturn (owning-vector overload) produces correct values", "[Pool][LoopReturn]")
{
	Pool pool(4);
	constexpr size_t N = 200;

	std::vector<int> result = pool.LoopReturn<int>(N, [](size_t i) { return static_cast<int>(i * 2); });

	REQUIRE(result.size() == N);
	for (size_t i = 0; i < N; ++i)
	{
		REQUIRE(result[i] == static_cast<int>(i * 2));
	}
}

TEST_CASE("LoopReturn (holder overload) fills a pre-sized vector correctly", "[Pool][LoopReturn]")
{
	Pool pool(4);
	constexpr size_t N = 200;
	std::vector<int> holder(N);

	pool.LoopReturn<int>(holder, [](size_t i) { return static_cast<int>(i + 1); });

	for (size_t i = 0; i < N; ++i)
	{
		REQUIRE(holder[i] == static_cast<int>(i + 1));
	}
}

TEST_CASE("LoopReturn works identically under both distribution policies", "[Pool][LoopReturn]")
{
	Pool pool(4);
	constexpr size_t N = 300;
	auto seq = pool.LoopReturn<int>(N, [](size_t i) { return static_cast<int>(i * i); }, DistributionPolicy::Sequential);
	auto bal = pool.LoopReturn<int>(N, [](size_t i) { return static_cast<int>(i * i); }, DistributionPolicy::Balanced);

	REQUIRE(seq == bal);
}

// -----------------------------------------------------------------------
// Concurrency / thread-safety sanity check
// -----------------------------------------------------------------------

TEST_CASE("Independent LoopReturn iterations do not corrupt each other's output", "[Pool][LoopReturn][thread-safety]")
{
	// Each element writes a value derived purely from its own index; if the
	// pool were mis-synchronising work across threads, we'd expect to see
	// values leak between indices intermittently. Run this a number of
	// times to make flaky sharing bugs more likely to surface.
	Pool pool(std::max(2u, std::thread::hardware_concurrency()));
	constexpr size_t N = 2000;

	for (int trial = 0; trial < 10; ++trial)
	{
		auto result = pool.LoopReturn<size_t>(N, [](size_t i) { return i; }, DistributionPolicy::Balanced);
		for (size_t i = 0; i < N; ++i)
		{
			REQUIRE(result[i] == i);
		}
	}
}

// -----------------------------------------------------------------------
// Soft performance requirement
// -----------------------------------------------------------------------
//
// This is a *soft* requirement, not a correctness check: timing tests are
// inherently sensitive to machine load, core count, and scheduler noise, so
// we do not want a slow CI runner to fail the build over this. We use a
// generous threshold and WARN rather than REQUIRE/FAIL on violation, so the
// signal is visible in test output without blocking the pipeline.

TEST_CASE("Parallel LoopReturn is faster than an equivalent serial loop for coarse-grained work", "[Pool][performance][!mayfail]")
{
	constexpr size_t N = 200;
	constexpr auto workPerTask = 2ms; // coarse-grained: dominated by work, not dispatch overhead

	auto workload = [](size_t i) -> size_t {
		std::this_thread::sleep_for(2ms);
		return i;
	};

	// Serial baseline
	auto serialStart = std::chrono::steady_clock::now();
	std::vector<size_t> serialResult(N);
	for (size_t i = 0; i < N; ++i)
	{
		serialResult[i] = workload(i);
	}
	auto serialDuration = std::chrono::steady_clock::now() - serialStart;

	// Parallel version
	unsigned int cores = std::max(2u, std::thread::hardware_concurrency());
	Pool pool(cores);

	auto parallelStart = std::chrono::steady_clock::now();
	auto parallelResult = pool.LoopReturn<size_t>(N, workload, DistributionPolicy::Balanced);
	auto parallelDuration = std::chrono::steady_clock::now() - parallelStart;

	// Correctness is a hard requirement regardless of timing.
	REQUIRE(parallelResult == serialResult);

	// Speed is a soft requirement: warn (don't fail the suite) if we don't
	// see at least some meaningful speedup. Threshold is deliberately
	// generous (1.5x, well under the theoretical `cores`x) to absorb
	// scheduling noise, thread startup cost, and shared/loaded CI machines.
	double speedup = std::chrono::duration<double>(serialDuration).count() /
					 std::chrono::duration<double>(parallelDuration).count();

	// INFO("cores = " << cores << ", serial = " << std::chrono::duration<double, std::milli>(serialDuration).count()
	// 				<< "ms, parallel = " << std::chrono::duration<double, std::milli>(parallelDuration).count()
	// 				<< "ms, speedup = " << speedup << "x");

	if (speedup < 1.5)
	{
		WARN("Parallel execution did not show the expected speedup over serial execution (soft requirement)");
	}
	CHECK(speedup > 1.0); // still worth flagging (as a normal, non-fatal CHECK) if parallel was outright slower
}
