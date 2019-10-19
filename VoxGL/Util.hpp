#pragma once

template<typename R, typename T = float>
struct TimedBlock {
  explicit TimedBlock(T &result): result(result) { }
  ~TimedBlock() { result = std::chrono::duration<T, R>(std::chrono::steady_clock::now() - start).count(); };

private:
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  T &result;
};

template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;
