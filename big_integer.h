//
// Created by taras on 13.04.19.
//

#ifndef BIG_INTEGER_HW__BIG_INTEGER_H_
#define BIG_INTEGER_HW__BIG_INTEGER_H_

#include <vector>
#include <cstddef>
#include <iosfwd>
#include <cstdint>
#include <functional>

struct big_integer {
  typedef uint32_t digit_type;
  big_integer();
  big_integer(big_integer const& other);
  big_integer(int x);
  big_integer(std::vector<digit_type> const& src, int flag);
  explicit big_integer(std::string const& str);
  ~big_integer() = default;

  big_integer& operator=(big_integer const& other);

  big_integer& operator+=(big_integer const& rhs);
  big_integer& operator-=(big_integer const& rhs);
  big_integer& operator*=(big_integer const& rhs);
  big_integer& operator/=(big_integer const& rhs);
  big_integer& operator%=(big_integer const& rhs);

  big_integer& operator&=(big_integer const& rhs);
  big_integer& operator|=(big_integer const& rhs);
  big_integer& operator^=(big_integer const& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(big_integer const& a, big_integer const& b);
  friend bool operator!=(big_integer const& a, big_integer const& b);
  friend bool operator<(big_integer const& a, big_integer const& b);
  friend bool operator>(big_integer const& a, big_integer const& b);
  friend bool operator<=(big_integer const& a, big_integer const& b);
  friend bool operator>=(big_integer const& a, big_integer const& b);

  friend std::string to_string(big_integer const& a);

 private:
  std::vector<digit_type> digits;
  int sign;
  static const uint64_t base = uint64_t(UINT32_MAX) + 1;
  static const digit_type max_digit = UINT32_MAX;
  static big_integer mul_by_short_with_shift(big_integer const& a, digit_type d, size_t shift);
  static std::pair<big_integer, digit_type> div_by_short(big_integer const& a, digit_type b);
  static std::pair<big_integer, big_integer> division(big_integer const& a, big_integer const& b);
  static int modulo_compare_to(big_integer const& a, big_integer const& b);
  big_integer make_binary_operation(big_integer const& a,
                                    big_integer const& b,
                                    std::function<digit_type(digit_type, digit_type)> const& f,
                                    std::function<int(int, int)> const& get_sign);
  big_integer get_reverse_code_with_add() const;
  big_integer get_twos_complement_code() const;
  big_integer get_sign_code() const;
  int sub_prefix(big_integer const& b, size_t first_ind);
  void add_prefix(big_integer const& b, size_t first_ind);
  void shrink_to_fit();
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif //BIG_INTEGER_HW__BIG_INTEGER_H_