// Copyright (c) 2016 by SN Systems Ltd., Sony Interactive Entertainment Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef ITER_PAIR_HPP
#define ITER_PAIR_HPP

template <typename Iterator>
class iter_pair {
public:
    iter_pair ()
            : begin_ ()
            , end_ () {}
    iter_pair (Iterator first, Iterator last)
            : begin_ (first)
            , end_ (last) {}
    Iterator begin () {
        return begin_;
    }
    Iterator end () {
        return end_;
    }

private:
    Iterator begin_;
    Iterator end_;
};
template <typename Iterator>
iter_pair<Iterator> make_range () {
    return {};
}
template <typename Iterator>
iter_pair<Iterator> make_range (Iterator begin, Iterator end) {
    return {begin, end};
}

#endif // ITER_PAIR_HPP
// eof iter_pair.hpp
