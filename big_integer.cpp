//
// Created by taras on 13.04.19.
//
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <climits>
#include <cstdlib>
#include <stdint-gcc.h>
#include <string.h>
#include <algorithm>

#include "big_integer.h"

//constructors
big_integer::big_integer() {
    sign = 0;
    digits.push_back(0);
}

big_integer::big_integer(big_integer const &other) {
    sign = other.sign;
    digits = other.digits;
}

big_integer::big_integer(int x) {
    if (x == 0) {
        sign = 0;
    } else {
        sign = x > 0 ? 1 : -1;
    }
    if (x == INT_MIN) {
        digits.push_back(2147483648u);
    } else {
        digits.push_back(abs(x));
    }
}

big_integer::big_integer(std::string const &str) : big_integer() {
    size_t i = (str[0] == '-') ? 1 : 0;
    while (i + 9 <= str.size()) {
        *this = *this * 1000000000 + std::stoi(str.substr(i, 9));
        i += 9;
    }
    if (i < str.size()) {
        int mul = 1;
        for (size_t j = i; j < str.size(); ++j) {
            mul *= 10;
        }
        *this = *this * mul + std::stoi(str.substr(i, str.size() - i));
    }
    sign = (str[0] == '-' ? -1 : 1);
    shrink_to_fit();
}

big_integer::big_integer(std::vector<digit_type> const &src, int flag) {
    digits = src;
    sign = flag;
}

//____________________________________________________________________________________________________________
//helpful function
void big_integer::shrink_to_fit() {
    while (digits.back() == 0 && digits.size() > 1) {
        digits.pop_back();
    }
    if (digits.size() == 1 && digits[0] == 0) {
        sign = 0;
    }
}

int big_integer::sub_prefix(big_integer const &b, size_t first_ind) {
    digit_type carry = 0;
    for (size_t i = 0; i < b.digits.size(); ++i) {
        if (digits[first_ind + i] - carry >= b.digits[i]) {
            digits[first_ind + i] = static_cast<digit_type>(digits[first_ind + i] - carry - b.digits[i]);
            carry = 0;
        } else {
            digits[first_ind + i] += base - carry - b.digits[i];
            carry = 1;
        }
    }
    if (carry != 0 && b.digits.size() < digits.size()) {
        if (digits[b.digits.size() + first_ind] != 0) {
            --digits[b.digits.size() + first_ind];
            carry = 0;
        } else {
            digits[b.digits.size() + first_ind] = base - 1;
        }
    }
    return carry;
}

void big_integer::add_prefix(big_integer const &b, size_t first_ind) {
    uint64_t carry = 0;
    for (size_t i = 0; i < b.digits.size(); ++i) {
        uint64_t new_digit = carry + digits[first_ind + i] + b.digits[i];
        digits[first_ind + i] = static_cast<digit_type>(new_digit % base);
        carry = new_digit >= base ? 1 : 0;
    }
    digits[first_ind + b.digits.size()] += carry;
}

big_integer big_integer::mul_by_short_with_shift(big_integer const &a, digit_type d, size_t shift) {
    if (d == 0)
        return big_integer();
    std::vector<digit_type> buf(a.digits.size() + shift);
    digit_type carry = 0;
    for (size_t i = 0; i < a.digits.size(); ++i) {
        uint64_t mul = static_cast<uint64_t>(a.digits[i]) * d + carry;
        buf[i + shift] = static_cast<digit_type>(mul % base);
        carry = mul >= base ? static_cast<digit_type>(mul / base) : 0;
    }
    if (carry != 0) {
        buf.push_back(carry);
    }
    big_integer res(buf, 1);
    res.shrink_to_fit();
    return res;
}

std::pair<big_integer, big_integer::digit_type> big_integer::div_by_short(big_integer const &a, digit_type b) {
    if (b == 0)
        throw std::runtime_error("Division by zero");
    digit_type r = 0;
    std::vector<digit_type> result(a.digits.size());
    for (size_t j = a.digits.size(); j-- > 0;) {
        uint64_t tmp = base * r + a.digits[j];
        result[j] = static_cast<digit_type>(tmp / b);
        r = static_cast<digit_type>(tmp % b);
    }
    big_integer quotient(result, a.sign);
    quotient.shrink_to_fit();
    return {quotient, r};
}

int big_integer::modulo_compare_to(big_integer const &a, big_integer const &b) {
    if (a.digits.size() > b.digits.size()) {
        return 1;
    }
    if (a.digits.size() < b.digits.size()) {
        return -1;
    }
    for (size_t i = a.digits.size(); i-- > 0;) {
        if (a.digits[i] > b.digits[i]) {
            return 1;
        } else if (b.digits[i] > a.digits[i]) {
            return -1;
        }
    }
    return 0;
}

big_integer big_integer::make_binary_operation(big_integer const &a,
                                               big_integer const &b,
                                               std::function<digit_type(digit_type, digit_type)> const &f,
                                               std::function<int(int, int)> const &get_sign) {
    big_integer big_bin, small_bin;
    if (a.digits.size() >= b.digits.size()) {
        big_bin = a.get_twos_complement_code();
        small_bin = b.get_twos_complement_code();
    } else {
        big_bin = b.get_twos_complement_code();
        small_bin = a.get_twos_complement_code();
    }
    std::vector<digit_type> buf(big_bin.digits.size());
    for (size_t i = 0; i < small_bin.digits.size(); ++i) {
        buf[i] = f(small_bin.digits[i], big_bin.digits[i]);
    }
    digit_type lead_block = small_bin.sign >= 0 ? 0 : max_digit;
    for (size_t i = small_bin.digits.size(); i < big_bin.digits.size(); ++i) {
        buf[i] = f(lead_block, big_bin.digits[i]);
    }
    big_integer result(buf, get_sign(a.sign, b.sign));
    result.shrink_to_fit();
    return result.get_sign_code();
}

big_integer big_integer::get_reverse_code_with_add() const {
    std::vector<digit_type> result(digits.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = ~digits[i];
    }
    return big_integer(result, 1) + 1;
}

big_integer big_integer::get_twos_complement_code() const {
    if (sign >= 0) {
        return *this;
    }
    return get_reverse_code_with_add();
}

big_integer big_integer::get_sign_code() const {
    if (sign >= 0) {
        return *this;
    }
    big_integer result(get_reverse_code_with_add());
    result.sign = sign;
    return result;
}

//____________________________________________________________________________________________________________
//print format

std::string to_string(big_integer const &a) {
    big_integer buf(a);
    std::string result;
    do {
        std::pair<big_integer, big_integer::digit_type> p = big_integer::div_by_short(buf, 10);
        result.push_back(static_cast<char> (p.second + '0'));
        buf = p.first;
    } while (buf.digits.size() > 1 || buf.digits[0] != 0);
    if (a.sign == -1) {
        result.push_back('-');
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    s << to_string(a);
    return s;
}

//____________________________________________________________________________________________________________
//logical block
bool operator==(big_integer const &a, big_integer const &b) {
    if (a.sign != b.sign || a.digits.size() != b.digits.size()) return false;
    for (size_t i_a = 0, i_b = 0;
         i_a != a.digits.size() && i_b != b.digits.size(); ++i_a, ++i_b) {
        if (a.digits[i_a] != b.digits[i_b]) return false;
    }
    return true;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    if (a.sign == b.sign && a.digits.size() == a.digits.size()) {
        for (size_t it = a.digits.size(); it-- > 0;) {
            if (a.digits[it] != b.digits[it])
                return a.digits[it] < b.digits[it] && a.sign >= 0;
        }
    } else {
        if (a.sign == b.sign)
            return a.digits.size() < b.digits.size();
        else if (a.sign == 0)
            return b.sign > 0;
        else if (b.sign == 0)
            return a.sign < 0;
        else return a.sign < 0 && b.sign > 0;
    }
    return false;
}

bool operator>(big_integer const &a, big_integer const &b) {
    if (a.sign == b.sign && a.digits.size() == a.digits.size()) {
        for (size_t it = a.digits.size(); it-- > 0;) {
            if (a.digits[it] != b.digits[it])
                return a.digits[it] > b.digits[it] && a.sign >= 0;
        }
    } else {
        if (a.sign == b.sign)
            return a.digits.size() > b.digits.size();
        else if (a.sign == 0)
            return b.sign < 0;
        else if (b.sign == 0)
            return a.sign > 0;
        else return !(a.sign < 0 && b.sign > 0);
    }
    return false;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return !(a > b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    *this = make_binary_operation(*this,
                                  rhs,
                                  [](digit_type x, digit_type y) { return x & y; },
                                  [](int sa, int sb) { return (sa == -1 && sb == -1) ? -1 : 1; });
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    *this = make_binary_operation(*this,
                                  rhs,
                                  [](digit_type x, digit_type y) { return x | y; },
                                  [](int sa, int sb) { return (sa == -1 || sb == -1) ? -1 : 1; });
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    *this = make_binary_operation(*this,
                                  rhs,
                                  [](digit_type x, digit_type y) { return x ^ y; },
                                  [](int sa, int sb) { return (sa == -1 ^ sb == -1) ? -1 : 1; });
    return *this;
}

big_integer big_integer::operator~() const {
    big_integer result(get_twos_complement_code());
    for (digit_type &digit : result.digits) {
        digit = ~digit;
    }
    result.sign = -sign;
    result = result.get_sign_code();
    result.shrink_to_fit();
    return result;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs == 0)
        return *this;

    std::vector<digit_type> result;
    static const int bits_size = sizeof(digit_type) * 8;
    while (rhs >= bits_size) {
        result.push_back(0);
        rhs -= bits_size;
    }
    digit_type carry = 0;
    for (digit_type &elem: this->digits) {
        result.push_back((elem << rhs) + carry);
        carry = elem >> (bits_size - rhs);
    }
    result.push_back(carry);
    *this = big_integer(result, sign);
    this->shrink_to_fit();
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs == 0)
        return *this;

    static const int bit_size = sizeof(digit_type) * 8;
    auto pop_cnt = static_cast<size_t>(rhs / bit_size);
    rhs %= bit_size;
    if (pop_cnt >= this->digits.size()) {
        if (sign >= 0) {
            *this = big_integer();
            return *this;
        } else {
            *this = big_integer(std::vector<digit_type>(this->digits.size(), base - 1), -1);
            return *this;
        }
    }
    big_integer a_bin = get_twos_complement_code();
    if (sign >= 0) {
        a_bin.digits.push_back(0);
    } else {
        a_bin.digits.push_back(base - 1);
    }
    std::vector<digit_type> result;
    for (size_t i = pop_cnt; i < a_bin.digits.size() - 1; ++i) {
        digit_type carry = a_bin.digits[i + 1] << (bit_size - rhs);
        result.push_back((a_bin.digits[i] >> rhs) + carry);
    }
    for (size_t i = 0; i < pop_cnt; i++) {
        result.push_back(a_bin.digits.back());
    }
    *this = big_integer(result, sign).get_sign_code();
    this->shrink_to_fit();
    return *this;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}
//__________________________________________________________________________________________________________
//arithmetic block

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer result(*this);
    result.sign *= -1;
    return result;
}

big_integer &big_integer::operator=(big_integer const &other) = default;

big_integer &big_integer::operator+=(big_integer const &rhs) {
    big_integer b(rhs);
    if (sign < 0 && rhs.sign > 0) {
        *this = b -= (-*this);
        return *this;
    }
    if (sign > 0 && b.sign < 0) {
        *this = *this -= (-b);
        return *this;
    }

    size_t small_size = std::min(digits.size(), b.digits.size());
    size_t big_size = std::max(digits.size(), b.digits.size());
    std::vector<digit_type> buf(big_size);
    digit_type carry = 0;
    for (size_t i = 0; i < small_size; ++i) {
        uint64_t sum = static_cast<uint64_t>(digits[i]) + b.digits[i] + carry;
        buf[i] = static_cast<digit_type>(sum % base);
        carry = sum >= base ? 1 : 0;
    }
    big_integer const &c = (big_size == digits.size()) ? *this : b;
    for (size_t i = small_size; i < big_size; ++i) {
        uint64_t sum = uint64_t(c.digits[i]) + carry;
        buf[i] = static_cast<digit_type>(sum % base);
        carry = sum >= base ? 1 : 0;
    }
    if (carry != 0) {
        buf.push_back(carry);
    }
    *this = big_integer(buf, sign == 0 ? b.sign : sign);
    this->shrink_to_fit();
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (sign < 0 && rhs.sign > 0) {
        *this = -((-*this) += rhs);
        return *this;
    }
    if (sign > 0 && rhs.sign < 0) {
        *this = *this + (-rhs);
        return *this;
    }

    big_integer const *big, *small;

    int res_sign = 0;
    if (modulo_compare_to(*this, rhs) >= 0) {
        res_sign = sign;
        small = &rhs;
        big = this;
    } else {
        res_sign = -sign;
        small = this;
        big = &rhs;
    }
    std::vector<digit_type> buf(big->digits.size());
    digit_type carry = 0;
    for (size_t i = 0; i < small->digits.size(); ++i) {
        if (carry != 0 && big->digits[i] == 0) {
            buf[i] = max_digit - small->digits[i];
        } else {
            digit_type big_digit = big->digits[i] - carry;
            if (big_digit < small->digits[i]) {
                carry = 1;
                buf[i] = static_cast<digit_type>((base - small->digits[i]) + big_digit);
            } else {
                carry = 0;
                buf[i] = big_digit - small->digits[i];
            }
        }
    }
    for (size_t i = small->digits.size(); i < big->digits.size(); ++i) {
        if (carry != 0 && big->digits[i] == 0) {
            buf[i] = max_digit;
        } else {
            digit_type big_digit = big->digits[i] - carry;
            carry = 0;
            buf[i] = big_digit;
        }
    }
    *this = big_integer(buf, res_sign);
    this->shrink_to_fit();
    return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    big_integer const &small = digits.size() < rhs.digits.size() ? *this : rhs;
    big_integer const &big = digits.size() < rhs.digits.size() ? rhs : *this;
    big_integer ans;
    for (size_t i = 0; i < small.digits.size(); i++) {
        ans += mul_by_short_with_shift(big, small.digits[i], i);
    }
    ans.sign = sign * rhs.sign;
    ans.shrink_to_fit();
    *this = ans;
    return *this;
}

std::pair<big_integer, big_integer> big_integer::division(big_integer const &a, big_integer const &b) {
    if (b.sign == 0)
        throw std::runtime_error("Division by zero");
    if (modulo_compare_to(a, b) == -1)
        return {big_integer(), big_integer(a)};
    if (b.digits.size() == 1) {
        std::pair<big_integer, big_integer> p = div_by_short(a, b.digits.back());
        p.first.sign = b.sign * a.sign;
        p.second.sign = a.sign;
        p.first.shrink_to_fit();
        p.second.shrink_to_fit();
        return p;
    }

    auto d = static_cast<digit_type>(base / static_cast<uint64_t>(b.digits.back() + 1));
    big_integer dividend(mul_by_short_with_shift(a, d, 0));
    big_integer divisor(mul_by_short_with_shift(b, d, 0));
    size_t n = b.digits.size();
    size_t m = a.digits.size() - n;
    std::vector<digit_type> quotient(m + 1);
    if (dividend.digits.size() != n + m + 1) {
        dividend.digits.push_back(0);
    }
    for (size_t j = m + 1; j-- > 0;) {
        uint64_t tmp = dividend.digits[j + n] * base + dividend.digits[j + n - 1];
        uint64_t q = tmp / divisor.digits.back();
        uint64_t r = tmp % divisor.digits.back();
        while (q == base || (r < base && q * divisor.digits[n - 2] > base * r + dividend.digits[j + n - 2])) {
            --q;
            r += divisor.digits.back();
        }
        if (dividend.sub_prefix(mul_by_short_with_shift(divisor, static_cast<digit_type>(q), 0), j)) {
            --q;
            dividend.add_prefix(divisor, j);
        }
        quotient[j] = static_cast<digit_type>(q);
    }
    big_integer res_q(quotient, a.sign * b.sign);
    res_q.shrink_to_fit();
    std::vector<digit_type> buf(n);
    for (size_t i = 0; i < n; ++i) {
        buf[i] = dividend.digits[i];
    }
    big_integer res_mod
        (div_by_short(big_integer(buf, a.sign), d).first);
    res_mod.sign = a.sign;
    res_mod.shrink_to_fit();
    return {res_q, res_mod};
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    *this = division(*this, rhs).first;
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    *this = division(*this, rhs).second;
    return *this;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

//___________________________________________________________________________________________________________
//optional
big_integer &big_integer::operator++() {
    *this += 1;
    return *this;
}
big_integer big_integer::operator++(int) {
    big_integer result(*this);
    *this += 1;
    return result;
}

big_integer &big_integer::operator--() {
    *this -= 1;
    return *this;
}
big_integer big_integer::operator--(int) {
    big_integer result(*this);
    *this -= 1;
    return result;
}
