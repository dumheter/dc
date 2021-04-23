#include <dc/dtest.hpp>
#include <dc/time.hpp>
#include <atomic>

static void wasteTime()
{
	static const char* text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

	constexpr int iters = 10'000;
	for (int i=0; i<iters; ++i)
	{
		s64 acc = 0;
		const char* p = text;
		while (*p != 0)
		{
			if (*p > 54) acc += 1;
			else if (*p < 54) acc -= 1;
			else acc = 0;
            ++p;
		}
	}
}

DTEST(getTimeUs) {
  const u64 beforeUs = dc::getTimeUs();
  std::atomic_signal_fence(std::memory_order_seq_cst); //< prevent beforeUs from being reordered
  wasteTime();
  const u64 afterUs = dc::getTimeUs();
  const u64 net = afterUs - beforeUs;

  DASSERT_TRUE(net > 0);
}

DTEST(getTimeUsNoReorder) {
  const u64 beforeUs = dc::getTimeUsNoReorder();
  std::atomic_signal_fence(std::memory_order_seq_cst); //< prevent beforeUs from being reordered
  wasteTime();
  const u64 afterUs = dc::getTimeUsNoReorder();
  const u64 net = afterUs - beforeUs;

  DASSERT_TRUE(net > 0);
}

DTEST(timestamp) {
  const auto a = dc::makeTimestamp();
  std::atomic_signal_fence(std::memory_order_seq_cst); //< prevent beforeUs from being reordered
  wasteTime();
  const auto b = dc::makeTimestamp();
  DASSERT_TRUE(a.second < b.second);
}

DTEST(stopwatch) {
	dc::Stopwatch s; //< construction also starts the stopwatch
	std::atomic_signal_fence(std::memory_order_seq_cst); //< prevent beforeUs from being reordered
	wasteTime();
	const u64 nowUs = s.nowUs();
	s.stop();
	DASSERT_TRUE(nowUs > 0);
	DASSERT_TRUE(s.us() > 0);
}
