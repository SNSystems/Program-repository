/*
  Independent implementation of MD5 (RFC 1321).

  This code implements the MD5 Algorithm defined in RFC 1321, whose
  text is available at
	http://www.ietf.org/rfc/rfc1321.txt
  The code is derived from the text of the RFC, including the test suite
  (section A.5) but excluding the rest of Appendix A.  It does not include
  any code or documentation that is identified in the RFC as being
  copyrighted.
*/
#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <stdlib.h>

/*
 * This package supports both compile-time and run-time determination of CPU
 * byte order.  If ARCH_IS_BIG_ENDIAN is defined as 0, the code will be
 * compiled to run only on little-endian CPUs; if ARCH_IS_BIG_ENDIAN is
 * defined as non-zero, the code will be compiled to run only on big-endian
 * CPUs; if ARCH_IS_BIG_ENDIAN is not defined, the code will be compiled to
 * run on either big- or little-endian CPUs, but will run slightly less
 * efficiently on either one than if ARCH_IS_BIG_ENDIAN is defined.
 */

typedef uint8_t  md5_byte_t;  /* 8-bit byte */
typedef uint32_t md5_word_t;  /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state {
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    uint8_t buf[64];		/* accumulate block */
} md5_state;

#ifdef __cplusplus
extern "C" 
{
#endif

    /* Initialize the algorithm. */
    void md5_init (md5_state *pms);

    /* Append a string to the message. */
    void md5_append (md5_state * const pms, void const *data, size_t nbytes);

    /* Finish the message and return the digest. */
    void md5_finish (md5_state * const pms, uint8_t digest [16]);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
    #include <algorithm>
    #include <array>
    #include <cstdint>
    #include <iosfwd>
    #include <ostream>
    #include <string>

    namespace md5 {
        typedef std::array <std::uint8_t, 16>  digest;

        class hasher {
        public:
            hasher () {
                ::md5_init (&state_);
            }

            void append (void const * ptr, std::size_t size) {
                ::md5_append (&state_, ptr, size);
            }
            
            template <typename T>
                void append (T const & t) {
                    this->append (&t, sizeof (t));
                }

            template <typename InputIterator>
                void append (InputIterator begin, InputIterator end) {
                    std::for_each (begin, end, [this] (typename InputIterator::value_type c) {
                        this->append (c);
                    });
                }

            digest finish () {
                digest result;
                ::md5_finish (&state_, result.data ());
                return result;
            }

        private:
            md5_state state_;
        };

        template <>
            inline void hasher::append (std::string const & str) {
                this->append (str.begin (), str.end ());
            }

        inline std::ostream & operator<< (std::ostream & os, digest const & digest) {
            auto hex_char = [] (std::uint8_t v) -> char {
                return static_cast <char> (v + (v < 10 ? '0' : 'A' - 10));
            };
            for (std::uint8_t v : digest) {
                os << hex_char ((v >> 4) & 0x0F) << hex_char (v & 0x0F);
            }
            return os;
        }
    } // namespace md5
#endif

#endif /* MD5_H */
// eof md5.h
