/* C++ code produced by gperf version 3.0.3 */
/* Command-line: /Library/Developer/CommandLineTools/usr/bin/gperf --compare-strncmp -t -L C++ keywords.gperf  */
/* Computed positions: -k'2' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 10 "keywords.gperf"
struct KeywordEntry { const char* name; TokenType token; };

#define TOTAL_KEYWORDS 16
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 6
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 25
/* maximum key range = 24, duplicates = 0 */

class KeywordLookup
{
private:
  static inline unsigned int keyword_hash (const char *str, unsigned int len);
public:
  static const struct KeywordEntry *lookup_keyword (const char *str, unsigned int len);
};

inline unsigned int
KeywordLookup::keyword_hash (const char *str, unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 20, 26, 26,
      26,  0,  0, 26, 15, 15, 26, 26, 10, 26,
      10,  5, 26, 26,  5, 26, 26,  0, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
      26, 26, 26, 26, 26, 26
    };
  return len + asso_values[(unsigned char)str[1]];
}

static const struct KeywordEntry wordlist[] =
  {
    {""}, {""},
#line 19 "keywords.gperf"
    {"if", TokenType::IF},
#line 18 "keywords.gperf"
    {"fun", TokenType::FUN},
    {""},
#line 24 "keywords.gperf"
    {"super", TokenType::SUPER},
#line 23 "keywords.gperf"
    {"return", TokenType::RETURN},
#line 21 "keywords.gperf"
    {"or", TokenType::OR},
#line 17 "keywords.gperf"
    {"for", TokenType::FOR},
#line 26 "keywords.gperf"
    {"true", TokenType::TRUE},
#line 22 "keywords.gperf"
    {"print", TokenType::PRINT},
    {""}, {""},
#line 13 "keywords.gperf"
    {"and", TokenType::AND},
#line 15 "keywords.gperf"
    {"else", TokenType::ELSE},
#line 14 "keywords.gperf"
    {"class", TokenType::CLASS},
    {""}, {""},
#line 20 "keywords.gperf"
    {"nil", TokenType::NIL},
#line 25 "keywords.gperf"
    {"this", TokenType::THIS},
#line 28 "keywords.gperf"
    {"while", TokenType::WHILE},
    {""}, {""},
#line 27 "keywords.gperf"
    {"var", TokenType::VAR},
    {""},
#line 16 "keywords.gperf"
    {"false", TokenType::FALSE}
  };

const struct KeywordEntry *
KeywordLookup::lookup_keyword (const char *str, unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = keyword_hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
