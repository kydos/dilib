#ifndef SRC_DILIB_MAYBE_HXX_
#define SRC_DILIB_MAYBE_HXX_

#include <iostream>
#include <functional>

namespace dilib {
  template <typename T>
  class Maybe;

  template <typename T>
  Maybe<T> Nothing();
}


template <typename T>
class dilib::Maybe {
public:
  Maybe() : empty_(true) { }
  explicit Maybe(const T& t) : empty_(false), v_(t) { }

  T fromJust() const {
    if (!empty_)  return v_;
    else  throw "Cannot get value from Nothing";
  }

  bool isNothing() const { return empty_; }

  bool isJust() const{ return !empty_; }

  static bool isJust(Maybe &m) { return m.isJust(); }

  static bool isNothing(Maybe &m) { return m.isNothing(); }

  template <typename Q>
  Maybe<Q> map(std::function<Q(T)> f) {
    if (this->isJust()) return Maybe<Q>(f(v_));
    else return Maybe<Q>();
  }
private:
  bool empty_;
  T v_;
};

template <typename T>
dilib::Maybe<T> dilib::Nothing() {
  return Maybe<T>();
}


template <typename T>
std::ostream& operator<<(std::ostream& s, const dilib::Maybe<T> m)
{
  if (m.isJust()) {
    return s << "Just " << m.fromJust();
  } else {
    return s << "Nothing";
  }
}



#endif /* SRC_DILIB_MAYBE_HXX_ */
