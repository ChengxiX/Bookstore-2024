#ifndef binable_hpp
#define binable_hpp

#include <concepts>

template<class T>
concept to_bin = requires(T t) {
    { t.to_bin() } -> std::same_as<char*>;
};
template<class T>
concept from_bin = requires(T t, char* s) {
    { t.from_bin(s) } -> std::same_as<void>;
};
template<class T>
concept bin_size = requires(T t) {
    { t.bin_size() } -> std::same_as<int>;
};
template<class T>
concept binable = to_bin<T> && from_bin<T> && bin_size<T>;

#endif