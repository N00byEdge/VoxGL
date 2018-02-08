#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <map>

#define ADDOPT(var) addOption(#var, var)

template<typename T>
void Read(std::istream &is, T &target);

template<typename T>
void Write(std::ostream &os, T const &source);

struct Config {
  explicit Config(std::string const &path);

  struct OptionBase {
    virtual ~OptionBase() = default;
    friend Config;
    virtual void read(std::istream &is) = 0;
    virtual void write(std::ostream &os) = 0;
  };

  template<typename T>
  struct Option: public OptionBase {
    explicit Option(T);
    T &operator()() { return value; }
    explicit operator T() { return value; }
  protected:
    T value;
    void read(std::istream &is) override;
    void write(std::ostream &os) override;
  };

  void addOption(std::string const &name, OptionBase &);
  void read();
  void write();

private:
  std::string path;
  std::unordered_map<std::string, OptionBase *> options;
};

template<typename T>
Config::Option<T>::Option(T arg): value(std::forward<T>(arg)) {}

template<typename T, typename ... Args>
Config::Option<T> MakeOption(Args &&... arg) { return Config::Option<T>(T(arg...)); }

template<typename T>
void Read(std::istream &is, std::vector<T> &target) {
  T val;
  size_t size;
  is >> size;
  target.resize(size);
  while(size -- > 0) {
    Read(is, val);
    target[size] = val;
  }
}

template<typename T1, typename T2>
void Read(std::istream &is, std::pair<T1, T2> &target) { is >> target.first >> target.second; }

template<typename T, size_t S>
void Read(std::istream &is, std::array<T, S> &target) {
  for(auto i = 0ull; i < S; ++i)
    is >> target[i];
}

template<typename T>
void Read(std::istream &is, T &target) { is >> target; }

template<typename T>
void Write(std::ostream &os, std::vector<T> const &source) {
  os << source.size() << " ";
  for(size_t i = 0; i < source.size(); ++i) {
    T val      = source[i];
    Write(os, val);
    if(i != source.size() - 1)
      os << " ";
  }
}

template<typename T1, typename T2>
void Write(std::ostream &os, std::pair<T1, T2> const &source) {
  Write(os, source.first);
  os << " ";
  Write(os, source.second);
}

template<typename T, size_t S>
void Write(std::ostream &os, std::array<T, S> const &source) {
  for(auto i = 0ull; i < S - 1; ++i)
    Write(os, source[i]), os << " ";
  if(source.size())
    Write(os, source[source.size() - 1]);
}

template<typename T>
void Write(std::ostream &os, T const &source) { os << source; }

template<typename T>
void Config::Option<T>::read(std::istream &is) { Read(is, value); }

template<typename T>
void Config::Option<T>::write(std::ostream &os) { Write(os, value); }
