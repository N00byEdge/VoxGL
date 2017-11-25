#pragma once

#include <unordered_map>
#include <iostream>
#include <memory>
#include <string>
#include <map>

#define ADDOPT(var) addOption(#var, var)

template <typename T>
void read(std::istream &is, T &target);

template <typename T>
void write(std::ostream &os, const T &source);

struct Config {
	Config(std::string path);

	struct OptionBase {
		virtual ~OptionBase() { }
		friend Config;
		virtual void read(std::istream &is) = 0;
		virtual void write(std::ostream &os) = 0;
	};

	template <typename T>
	struct Option : public Config::OptionBase {
		Option(T);
		virtual ~Option() { }
		inline T &operator()() { return value; }
		inline operator T() { return value; }
	protected:
		T value;
		void read(std::istream &is) override;
		void write(std::ostream &os) override;
	};

	void addOption(std::string name, OptionBase &);
	void read();
	void write();

private:
	std::string path;
	std::unordered_map<std::string, OptionBase *> options;
};

template<typename T>
Config::Option<T>::Option(T arg): value(std::forward<T>(arg)) {

}

template<typename T, typename ... Args>
Config::Option<T> makeOption(Args&&... arg) {
	return Config::Option<T>(T(arg...));
}

template <typename T>
void read(std::istream &is, std::vector <T> &target) {
	T val;
	size_t size;
	is >> size;
	target.resize(size);
	while (size --> 0) {
		read(is, val);
		target[size] = val;
	}
}

template <typename T1, typename T2>
void read(std::istream &is, std::pair<T1, T2> &target) {
	is >> target.first >> target.second;
}

template <typename T, size_t S>
void read(std::istream &is, std::array<T, S> &target) {
	for (auto i = 0ull; i < S; ++i)
		is >> target[i];
}

template <typename T>
void read(std::istream &is, T &target) {
	is >> target;
}

template <typename T>
void write(std::ostream &os, const std::vector <T> &source) {
	T val;
	os << source.size() << " ";
	for (size_t i = 0; i < source.size(); ++i) {
		val = source[i];
		write(os, val);
		if (i != source.size() - 1) os << " ";
	}
}

template <typename T1, typename T2>
void write(std::ostream &os, const std::pair<T1, T2> &source) {
	write(os, source.first);
	os << " ";
	write(os, source.second);
}

template <typename T, size_t S>
void write(std::ostream &os, const std::array<T, S> &source) {
	for (auto i = 0ull; i < S - 1; ++i)
		write(os, source[i]), os << " ";
	if(source.size()) write(os, source[source.size() - 1]);
}

template <typename T>
void write(std::ostream &os, const T &source) {
	os << source;
}

template <typename T>
void Config::Option<T>::read(std::istream &is) {
	::read(is, value);
}

template <typename T>
void Config::Option<T>::write(std::ostream &os) {
	::write(os, value);
}
