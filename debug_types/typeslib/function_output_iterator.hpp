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

#ifndef FUNCTION_OUTPUT_ITERATOR_HPP
#define FUNCTION_OUTPUT_ITERATOR_HPP

#include <iterator>

template <class UnaryFunction>
class function_output_iterator {
public:
    typedef std::output_iterator_tag iterator_category;
    typedef void value_type;
    typedef void difference_type;
    typedef void pointer;
    typedef void reference;

    function_output_iterator () {}
    explicit function_output_iterator (UnaryFunction const & f)
            : function_ (f) {}
    function_output_iterator (function_output_iterator const & rhs)
            : function_ (rhs.function_) {}

    function_output_iterator & operator= (function_output_iterator const & rhs) {
        function_ = rhs.function_;
        return *this;
    }

    struct output_proxy {
        output_proxy (UnaryFunction & f)
                : function_ (f) {}

        template <class T>
        output_proxy & operator= (T const & value) {
            function_ (value);
            return *this;
        }

        UnaryFunction & function_;
    };

    output_proxy operator* () {
        return output_proxy (function_);
    }

    function_output_iterator & operator++ () {
        return *this;
    }
    function_output_iterator & operator++ (int) {
        return *this;
    }

private:
    UnaryFunction function_;
};

template <class UnaryFunction>
inline auto make_function_output_iterator (UnaryFunction const & f = UnaryFunction ())
    -> function_output_iterator<UnaryFunction> {
    return function_output_iterator<UnaryFunction> (f);
}

#endif // FUNCTION_OUTPUT_ITERATOR_HPP
