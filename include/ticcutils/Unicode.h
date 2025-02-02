#ifndef TICC_UNICODE_H
#define TICC_UNICODE_H

/*
  Copyright (c) 2006 - 2024
  CLST  - Radboud University
  ILK   - Tilburg University

  This file is part of ticcutils

  ticcutils is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  ticcutils is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.

  For questions and suggestions, see:
      https://github.com/LanguageMachines/ticcutils/issues
  or send mail to:
      lamasoftware (at ) science.ru.nl

*/

#include <cstddef>
#include <string>
#include <vector>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include "unicode/umachine.h"
#include "unicode/uversion.h"
#include "unicode/unistr.h"
#include "unicode/translit.h"
#include "unicode/ustream.h"
#include "unicode/normalizer2.h"
#include "unicode/regex.h"

namespace TiCC {
  using namespace icu;

  std::string UnicodeToUTF8( const UnicodeString&,
			     const std::string& = "" );

  UnicodeString UnicodeFromEnc( const std::string&,
				const std::string& = "UTF8",
				const std::string& = "" );

  UnicodeString UnicodeFromUTF8( const std::string&,
				 const std::string& = "" );

  /// \brief a class that can normalize UnicodeStrings to NFC/NFD/NFKC/NFKD
  class UnicodeNormalizer {
  public:
    explicit UnicodeNormalizer( const std::string& = "" );
    explicit UnicodeNormalizer( const char * str ):
      UnicodeNormalizer( std::string(str) ){};
    ~UnicodeNormalizer();
    UnicodeString normalize( const UnicodeString& );
    const std::string setMode( const std::string& );
    const std::string& getMode() const { return _mode; };
  private:
    const Normalizer2 *_normalizer;
    std::string _mode;
  };

  /// \brief a class that can match UnicodeStrings to Regex patterns
  class UnicodeRegexMatcher {
  public:
    explicit UnicodeRegexMatcher( const UnicodeString&,
				  const UnicodeString& name="" );
    ~UnicodeRegexMatcher();
    bool match_all( const UnicodeString&, UnicodeString&, UnicodeString&  );
    const UnicodeString get_match( unsigned int ) const;
    int NumOfMatches() const;
    int split( const UnicodeString&, std::vector<UnicodeString>& );
    UnicodeString Pattern() const;
    bool set_debug( bool b ){ bool r = _debug; _debug = b; return r; };
  private:
     // inhibit copies!
    UnicodeRegexMatcher( const UnicodeRegexMatcher& ) = delete;
    UnicodeRegexMatcher& operator=( const UnicodeRegexMatcher& ) = delete;
    RegexPattern *_pattern;
    RegexMatcher *_matcher;
    UnicodeRegexMatcher();
    std::vector<UnicodeString> _results;
    const UnicodeString _name;
    bool _debug;
  };

  /// \brief a class to run ICU Unicode filters on UnicodeStrings
  class UniFilter {
    friend std::ostream& operator<<( std::ostream&, const UniFilter& );
  public:
    UniFilter();
    ~UniFilter();
    bool init( const UnicodeString&, const UnicodeString& );
    bool is_initialized() const { return _trans != 0; };
    bool fill( const std::string&, const std::string& = "" );
    bool add( const std::string& );
    bool add( const UnicodeString& );
    UnicodeString filter( const UnicodeString& );
    UnicodeString get_rules() const;
  private:
    Transliterator *_trans;
  };

  UnicodeString filter_diacritics( const UnicodeString& );

  std::vector<UnicodeString> split_at( const UnicodeString&,
				       const UnicodeString&,
				       size_t = 0 );

  std::vector<UnicodeString> split_at_first_of( const UnicodeString&,
						const UnicodeString&,
						size_t = 0 );

  std::vector<UnicodeString> split( const UnicodeString&,
				    size_t = 0 );

  std::vector<icu::UnicodeString> split_exact_at( const icu::UnicodeString&,
						  const icu::UnicodeString& );
  std::vector<icu::UnicodeString> split_exact_at_first_of( const icu::UnicodeString&,
							   const icu::UnicodeString& );
  inline  std::vector<icu::UnicodeString> split_exact( const icu::UnicodeString& s ){
    return split_exact_at_first_of( s, " \r\t\n" );
  }

  UnicodeString utrim( const UnicodeString&, const UnicodeString& = "\r\n\t " );
  UnicodeString ltrim( const UnicodeString&, const UnicodeString& = "\r\n\t " );
  UnicodeString rtrim( const UnicodeString&, const UnicodeString& = "\r\n\t " );
  UnicodeString pad( const UnicodeString&, int, const UChar32 = ' ' );
  std::string utf8_lowercase( const std::string& ); // Unicode safe version
  std::string utf8_uppercase( const std::string& ); // Unicode safe version

  std::istream& getline( std::istream&,
			 icu::UnicodeString&,
			 const std::string&,
			 const char = '\n' );

  std::istream& getline( std::istream&,
			 icu::UnicodeString&,
			 const char = '\n' );

  template< typename T >
    inline T stringTo( const icu::UnicodeString& str ) {
    T result;
    std::string tmp = TiCC::UnicodeToUTF8(str);
    std::stringstream dummy( tmp );
    if ( !( dummy >> result ) ) {
      throw( std::runtime_error( "conversion from string '" + tmp + "' to type:"
				 + typeid(result).name() + " failed" ) );
    }
    return result;
  }

  template< typename T >
    inline bool stringTo( const icu::UnicodeString& str, T& result ) {
    try {
      result = stringTo<T>( str );
      return true;
    }
    catch( ... ){
     return false;
    }
  }

  template< typename T >
    inline icu::UnicodeString toUnicodeString ( const T& obj ) {
    std::stringstream dummy;
    if ( !( dummy << obj ) ) {
      throw( std::runtime_error( std::string("conversion from type:")
				 + typeid(obj).name()
				 + " to UnicodeString failed" ) );
    }
    return TiCC::UnicodeFromUTF8(dummy.str());
  }

  template<>
  // specialization for std::string
  inline icu::UnicodeString toUnicodeString ( const std::string& obj ) {
    return TiCC::UnicodeFromUTF8(obj);
  }

  template< typename T>
  inline icu::UnicodeString join( const std::vector<T>& vec,
				  const icu::UnicodeString& sep = " " ){
    icu::UnicodeString result;
    for ( const auto& it : vec ){
      if ( &it != &(*vec.begin() ) ){
	result += sep;
      }
      result += TiCC::toUnicodeString(it);
    }
    return result;
  }

  template<>
  /// specialization for UnicodeString
  inline icu::UnicodeString join( const std::vector<icu::UnicodeString>& vec,
				  const icu::UnicodeString& sep ){
    icu::UnicodeString result;
    for ( const auto& it : vec ){
      if ( &it != &(*vec.begin() ) ){
	result += sep;
      }
      result += it;
    }
    return result;
  }

  UnicodeString format_non_printable( const UChar32 );
  UnicodeString format_non_printable( const UnicodeString& );

}
#endif // TICC_UNICODE_H
