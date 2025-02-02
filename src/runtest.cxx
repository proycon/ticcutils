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

#include <string>
#include "config.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdexcept>

#include "ticcutils/StringOps.h"
#include "ticcutils/UniHash.h"
#include "ticcutils/PrettyPrint.h"
#include "ticcutils/zipper.h"
#include "ticcutils/Version.h"
#include "ticcutils/UnitTest.h"
#include "ticcutils/FileUtils.h"
#include "ticcutils/CommandLine.h"
#include "ticcutils/Configuration.h"
#include "ticcutils/Timer.h"
#include "ticcutils/LogStream.h"
#include "ticcutils/Unicode.h"
#include "ticcutils/json.hpp"
#include "ticcutils/enum_flags.h"
#include "ticcutils/XMLtools.h"

using namespace std;
using namespace TiCC;
using namespace icu;

void helper(){
  throw runtime_error("expected_error");
}

int helper2(){
  throw runtime_error("mis");
}

void test_throw(){
  assertThrow( helper(), runtime_error );
  assertNoThrow( 4==7 );
}

void test_nothrow(){
  assertNoThrow( helper() );
  assertThrow( helper(), runtime_error );
}

void test_opt_1(){
  startTestSerie( "subtest 1" );
  bool opt_dbg=false;
  CL_Options opts1;
  opts1.set_debug(opt_dbg);
  opts1.allow_args( "t:fh" );
  // -t mist een optie
  assertThrow( opts1.parse_args( "-t -f -h" ), OptionError );
  // onbekende optie
  assertThrow( opts1.init( "-a" ), OptionError );
  // -f heeft onterecht een parameter ==> massopts
  assertNoThrow( opts1.parse_args( "-t1 -f bla -h") );
}

void test_opt_2(){
  startTestSerie( "subtest 2" );
  bool opt_dbg=false;
  CL_Options opts1;
  opts1.set_debug(opt_dbg);
  opts1.allow_args( "t:fh" );
  // -t mist een optie
  assertThrow( opts1.parse_args( "-t -f -h" ), OptionError );
  // onbekende optie
  assertThrow( opts1.init( "-a" ), OptionError );
  // -f heeft onterecht een parameter ==> massopts
  assertNoThrow( opts1.parse_args( "-t1 -f bla -h") );
}

void test_opt_3(){
  startTestSerie( "subtest 3" );
  CL_Options opts3;
  opts3.allow_args( "", "true:,false" );
  // - te weinig
  assertThrow( opts3.parse_args( "-true=false"), OptionError );
  // onbekende optie
  assertThrow( opts3.parse_args( "--magniet"), OptionError );
}

void test_opt_4(){
  startTestSerie( "subtest 4" );
  CL_Options opts4;
  opts4.allow_args( "", "true:,false" );
  // --true heeft optie, OK en
  // --false heeft geen optie --> massOpts.
  assertNoThrow( opts4.parse_args( "--true 1 --false 2")  );
  string value;
  opts4.is_present( "true", value );
  assertEqual( value, "1" );
}

void test_opt_5(){
  startTestSerie( "subtest 5" );
  CL_Options opts5( "", "false:,true::" );
  // --true heeft optionele optie,
  // --false heeft optie
  assertNoThrow( opts5.parse_args( "--true --false 2")  );
  string value;
  opts5.is_present( "true", value );
  assertEqual( value, "" );
}

void test_opt_6(){
  startTestSerie( "subtest 6" );
  string lo6 = "false:,true::";
  CL_Options opts6;
  opts6.allow_args( "", lo6 );
  assertEqual( opts6.get_long_options(), lo6 );
  // --true heeft optionele optie,
  // --false heeft optie
  assertNoThrow( opts6.parse_args( "--true ok --false=6")  );
  string value;
  opts6.is_present( "true", value );
  assertEqual( value, "ok" );
  int ival = 8;
  opts6.is_present( "false", ival );
  assertEqual( ival, 6 );
}

void test_opt_7(){
  startTestSerie( "subtest 7" );
  string so7 = "f:t::";
  CL_Options opts7;
  opts7.allow_args( so7 );
  assertEqual( opts7.get_short_options(), so7 );
  // -t heeft optionele optie,
  // -f heeft optie
  assertNoThrow( opts7.parse_args( "-t ok -f6")  );
  bool mood;
  string value;
  opts7.is_present( 't', value, mood );
  assertEqual( value, "ok" );
  opts7.is_present( 'f', value, mood );
  assertEqual( value, "6" );
}

void test_opt_8(){
  startTestSerie( "subtest 8" );
  CL_Options opts8( "t::,f:", "" );
  // -t heeft optionele optie,
  // -f heeft optie
  assertNoThrow( opts8.parse_args( "-t -f6")  );
  string value;
  bool mood;
  opts8.is_present( 't', value, mood );
  assertEqual( value, "" );
  opts8.is_present( 'f', value, mood );
  assertEqual( value, "6" );
}

void test_opt_9(){
  startTestSerie( "subtest 9" );
  CL_Options opts9;
  opts9.allow_args( "t::qp:r:" );
  // -t heeft optionele optie. q is een stoorzender
  assertNoThrow( opts9.parse_args( "-t 1 -t2 -t3 -q -t -t4 -p5 -r appel ")  );
  vector<string> ts;
  string value;
  bool mood;
  while ( opts9.extract( 't', value, mood ) ){
    ts.push_back( value );
  }
  assertEqual( ts.size() , 5 );
  assertEqual( ts[0], "1" );
  assertEqual( ts[1], "2" );
  assertEqual( ts[2], "3" );
  assertEqual( ts[3], "" );
  assertEqual( ts[4], "4" );
  assertTrue( opts9.is_present('q') );
  assertTrue( opts9.extract('q') );
  assertFalse( opts9.extract('q') );
  int myint = -1;
  assertTrue( opts9.is_present('p', myint ) );
  assertEqual( myint, 5 );
  myint = -1;
  assertTrue( opts9.extract('p', myint ) );
  assertEqual( myint, 5 );
  assertThrow( opts9.extract('r', myint ), OptionError );
}

void test_opt_10(){
  startTestSerie( "subtest 10" );
  CL_Options opts10;
  opts10.allow_args( "", "test::,qed,data:" );
  bool opt_dbg=false;
  opts10.set_debug(opt_dbg);
  // --test heeft optionele optie. qed is een stoorzender
  assertNoThrow( opts10.parse_args( "--test 1 --test=2 --qed --test --test=3 --data=5.6 --data=appel")  );
  vector<string> ts;
  string value;
  while ( opts10.extract( "test", value ) ){
    ts.push_back( value );
  }
  assertEqual( ts.size() , 4 );
  assertEqual( ts[0], "1" );
  assertEqual( ts[1], "2" );
  assertEqual( ts[2], "" );
  assertEqual( ts[3], "3" );
  assertTrue( opts10.is_present("qed") );
  assertTrue( opts10.extract("qed") );
  assertFalse( opts10.extract("q") );
  double mydouble = -3.14;
  assertTrue( opts10.is_present("data", mydouble ) );
  assertEqual( mydouble, 5.6 );
  mydouble = -3.14;
  assertTrue( opts10.extract("data", mydouble ) );
  assertEqual( mydouble, 5.6 );
  assertThrow( opts10.extract("data", mydouble ), OptionError );
}

void test_opt_11(){
  startTestSerie( "subtest 11" );
  CL_Options opts11;
  opts11.allow_args( "", "test:" );
  opts11.parse_args( "--test=test/a arg1" );
  string ex;
  opts11.extract( "test", ex );
  assertEqual( ex, "test/a" );
}

void test_opt_12(){
  startTestSerie( "subtest 12" );
  CL_Options opts12;
  opts12.allow_args( "a:", "a:" );
  opts12.parse_args( "-a 1 --a=2 a aa" );
  string ex;
  opts12.extract( 'a', ex );
  assertEqual( ex, "1" );
  opts12.extract( "a", ex );
  assertEqual( ex, "2" );
  vector<string> mo = opts12.getMassOpts();
  assertEqual( mo.size(), 2 );
  assertEqual( mo[0], "a" );
  assertEqual( mo[1], "aa" );
}

void test_opt_13(){
  startTestSerie( "subtest 13" );
  CL_Options opts13;
  opts13.parse_args( "-a b -a c oke -dfiets --appel peer --fout=goed toch" );
  assertEqual( opts13.toString(), "-a b -a c -d fiets --appel=peer --fout=goed" );
  auto v = opts13.getMassOpts();
  assertEqual( v.size(), 2 );
}

void test_opt_14(){
  startTestSerie( "subtest 14" );
  CL_Options opts14;
  bool opt_dbg=false;
  opts14.set_debug(opt_dbg);
  opts14.parse_args( "-a b -a c oke -d\"-fiets --appel peer \" --fout=goed toch" );
  assertEqual( opts14.toString(), "-a b -a c -d -fiets --appel peer  --fout=goed" );
  auto v = opts14.getMassOpts();
  assertEqual( v.size(), 2 );
}

void test_opt_15(){
  startTestSerie( "subtest 15" );
  CL_Options opts15;
  bool opt_dbg=false;
  opts15.set_debug(opt_dbg);
  opts15.parse_args( "--fout=goed\\mis --jan=gek" );
  assertEqual( opts15.toString(), "--fout=goed\\mis --jan=gek" );
  string res;
  opts15.extract("fout", res );
  assertEqual( res, "goed\\mis" );
  opts15.extract("jan", res );
  assertEqual( res, "gek" );
}

void test_opt_16(){
  startTestSerie( "subtest 16" );
  CL_Options opts16;
  opts16.allow_args( "", "test:" );
  bool opt_dbg=false;
  opts16.set_debug(opt_dbg);
  opts16.parse_args( "--test goed --test=prima --test niet=eens --test=wel=eens" );
  string res;
  opts16.extract("test", res );
  assertEqual( res, "goed" );
  opts16.extract("test", res );
  assertEqual( res, "prima" );
  opts16.extract("test", res );
  assertEqual( res, "niet=eens" );
  opts16.extract("test", res );
  assertEqual( res, "wel=eens" );
}

void test_opt_17(){
  startTestSerie( "subtest 17" );
  CL_Options opts17;
  // new feature: check for stray mass opts inside commandline
  opts17.allow_args( "ab:c", "aap" );
  opts17.set_debug(false);
  opts17.parse_args( "-a file1 -b prima de luxe --aap file2 -c file3 file4" );
  string res;
  opts17.extract("a", res );
  assertEqual( res, "" );
  opts17.extract("b", res );
  assertEqual( res, "prima" );
  opts17.extract("aap", res );
  assertEqual( res, "" );
  opts17.extract("c", res );
  assertEqual( res, "" );
  vector<string> mo2 = opts17.getMassOpts();
  assertEqual( mo2.size(), 6  );
}

void test_opt_18(){
  startTestSerie( "subtest 18" );
  CL_Options opts;
  // check bug: space after '=' in long option
  opts.allow_args( "", "aap:" );
  opts.set_debug(true);
  opts.parse_args( "--aap value1 --aap=value2 test --aap= value3" );
  vector<string> ts;
  string res;
  while ( opts.extract("aap", res ) ){
    ts.push_back(res);
  }
  assertEqual( ts.size() , 3 );
  assertEqual( ts[0], "value1" );
  assertEqual( ts[1], "value2" );
  assertEqual( ts[2], "value3" );
  auto mv = opts.getMassOpts();
  assertEqual( mv.size(), 1 );
  assertEqual( mv[0], "test" );
}

void test_opts_basic(){
  startTestSerie( "we testen basic commandline opties." );
  test_opt_1();
  test_opt_2();
  test_opt_3();
  test_opt_4();
  test_opt_5();
  test_opt_6();
  test_opt_7();
  test_opt_8();
  test_opt_9();
  test_opt_10();
  test_opt_11();
  test_opt_12();
  test_opt_13();
  test_opt_14();
  test_opt_15();
  test_opt_16();
  test_opt_17();
  test_opt_18();
}

void test_opts( CL_Options& opts ){
  startTestSerie( "we testen nog meer commandline opties." );
  // opts.dump(cerr);
  // cerr << endl;
  string value;
  bool pol;
  opts.is_present( 't', value, pol );
  assertEqual( value, "true" );
  assertEqual( pol, true );
  opts.is_present( 'f', value, pol );
  assertEqual( value, "false" );
  assertEqual( pol, false );
  opts.is_present( 'd', value, pol );
  assertTrue( value != "" );
  opts.is_present( "test", value );
  assertEqual( value, "test" );
  opts.is_present( "raar", value );
  assertEqual( value, "" );
  vector<string> mo = opts.getMassOpts();
  assertTrue( mo.size() == 3 );
  assertEqual( mo[0], "blaat" );
  assertEqual( mo[1], "arg1" );
  assertEqual( mo[2], "arg2" );
}

void test_subtests_fail(){
  startTestSerie( "we testen subtests, met faal." );
  assertThrow( helper(), range_error );
  assertEqual( helper2(), 4 );
  assertEqual( (1 + 2), (2 + 1) );
  assertEqual( 4, 5 );
}

void test_subtests_ok(){
  startTestSerie( "we testen subtests, allemaal OK." );
  assertThrow( helper(), runtime_error );
  assertEqual( 4, 4 );
  assertTrue( true );
}

void test_trim(){
  string val = " aha ";
  string res = trim(val);
  assertEqual( res, "aha" );
  assertEqual( "", trim(" \r ") );
  assertTrue( trim(" \r ").empty() );
  assertEqual( "A", trim("A") );
  assertEqual( "AHA", trim("AHA") );
  assertEqual( "AHA", trim("AHA\r\n") );
  res = trim(val,"a");
  assertEqual( res, " aha " );
  res = trim(val,"a ");
  assertEqual( res, "h" );
  res = trim( " \"test\""," \"");
  assertEqual( res, "test" );
}

void test_trim_front(){
  string val = " aha ";
  string res = trim_front(val);
  assertEqual( res, "aha " );
}

void test_trim_back(){
  string val = " aha ";
  string res = trim_back(val);
  assertEqual( res, " aha" );
}

void test_pad(){
  string val = "Q";
  string res = pad(val,10,'x');
  assertEqual( res, "xxxxxxxxxQ" );
  res = pad(val,5);
  assertEqual( res, "    Q" );
  UnicodeString u_val = "τ";
  UnicodeString u_res = pad( u_val,10,u'έ' );
  assertEqual( u_res, "έέέέέέέέέτ" );
  u_res = pad(u_val,5);
  assertEqual( u_res, "    τ" );
}

void test_match_front(){
  assertTrue( match_front("janklaassenenkatrien", "janklaassen" ) );
  assertFalse( match_front("janklaassenenkatrien", "anklaassen" ) );
  assertNoThrow( match_front("janklaassen", "janklaassenenkatrien" ) );
  assertFalse( match_front("janklaassen", "janklaassenenkatrien" ) );
}

void test_match_back(){
  assertTrue( match_back("janklaassenenkatrien", "katrien" ) );
  assertFalse( match_back("janklaassenenkatrien", "katrie" ) );
  assertNoThrow( match_back("katrien", "janklaassenenkatrien" ) );
  assertFalse( match_back("katrien", "janklaassenenkatrien" ) );
}

void test_format_non_printable(){
  string val = "ø en €";
  string res = format_non_printable(val);
  assertEqual( res, "-0xffc3--0xffb8- en -0xffe2--0xff82--0xffac-" );
  UnicodeString uval = "‌A";  // starts with a ZWNJ
  UnicodeString ures = format_non_printable( uval );
  assertEqual( ures, "-0x200c-A" );
  UChar32 uc = L'私';
  ures = format_non_printable( uc );
  assertEqual( ures, "-0x79c1-" );
  uc = U'\U00007982';
  ures = format_non_printable( uc );
  assertEqual( ures, "-0x7982-" );
}

void test_split(){
  string line = "De kat krabt de krullen\n van de   trap.";
  vector<string> res;
  int cnt = split( line, res );
  assertEqual( cnt, 8 );
  assertEqual( res[5], "van" );
  vector<string> res2 = split( line );
  assertEqual( res2.size(), 8 );
  assertEqual( res2[6], "de" );
  string outline = join( res2 );
  assertEqual( outline, "De kat krabt de krullen van de trap." );
  outline = join( res2, "\t" );
  assertEqual( outline, "De\tkat\tkrabt\tde\tkrullen\tvan\tde\ttrap." );
  vector<string> res3 = split( line, 3 );
  assertEqual( res3.size(), 3 );
  assertEqual( res3[1], "kat" );
  assertEqual( res3[2], "krabt de krullen\n van de   trap." );
  vector<string> res4 = split( line, 24 );
  assertEqual( res4.size(), 8 );
  assertEqual( res4[1], "kat" );
  assertEqual( res4[2], "krabt" );
  vector<string> res5 = split( line, -1 );
  assertEqual( res5.size(), 8 );
  assertEqual( res5[1], "kat" );
  assertEqual( res5[2], "krabt" );
  vector<string> res6 = split( line, 0 );
  assertEqual( res6.size(), 8 );
  assertEqual( res6[1], "kat" );
  assertEqual( res6[2], "krabt" );
  vector<string> res7 = split( string("APPELTAART"), 2 );
  assertEqual( res7.size(), 1 );
  assertEqual( res7[0], "APPELTAART" );
}

void test_split_exact(){
  string line = "1 2  4    8  10";
  vector<string> res;
  int cnt = split_exact( line, res );
  assertEqual( cnt, 10 );
  assertEqual( res[5], "" );
}

void test_split_at_exact(){
  string line = "1/2//4////8//10";
  vector<string> res;
  int cnt = split_exact_at( line, res, "/" );
  assertEqual( cnt, 10 );
  assertEqual( res[5], "" );
}

void test_split_at(){
  string line = "Derarekatrarekrabtrarederarekrullen\nrarevanrarederaretrap.";
  vector<string> res;
  int cnt = split_at( line, res, "rare" );
  assertEqual( cnt, 8 );
  assertEqual( res[5], "van" );
  vector<string> res2 = split_at( line, "rare" );
  assertEqual( res2.size(), 8 );
  assertEqual( res2[6], "de" );
  vector<string> res3 = split_at( line, "rare", 4 );
  assertEqual( res3.size(), 4 );
  assertEqual( res3[2], "krabt" );
  assertEqual( res3[3], "derarekrullen\nrarevanrarederaretrap." );
}

void test_split_at_first(){
  string line = "De.kat,krabt:de;krullen?van.,;.;de!trap.";
  vector<string> res;
  int cnt = split_at_first_of( line, res, ".,?!:;" );
  assertEqual( cnt, 8 );
  assertEqual( res[5], "van" );
  vector<string> res2 = split_at_first_of( line, ".,?!:;" );
  assertEqual( res2.size(), 8 );
  assertEqual( res2[6], "de" );
  vector<string> res3 = split_at_first_of( line, ".,?!:;", 7 );
  assertEqual( res3.size(), 7 );
  assertEqual( res3[4], "krullen" );
  assertEqual( res3[6], ",;.;de!trap." );
}

void test_split_at_first_exact(){
  string line = "De.kat,krabt:de;krullen?van.,;.;de!trap.";
  vector<string> res;
  int cnt = split_exact_at_first_of( line, res, ".,?!:;" );
  assertEqual( cnt, 13 );
  //  cerr << "after split: " << res << endl;
  assertEqual( res[5], "van" );
  assertEqual( res[9], "" );
}

void test_to_upper(){
  string line = "Een CamelCapped Zin.";
  to_upper( line );
  assertEqual( line, "EEN CAMELCAPPED ZIN." );
}

void test_to_lower(){
  string line = "Een CamelCapped Zin.";
  to_lower( line );
  assertEqual( line, "een camelcapped zin." );
}

void test_uppercase(){
  string line = "Een CamelCapped Zin.";
  string res = uppercase( line );
  assertEqual( res, "EEN CAMELCAPPED ZIN." );
}

void test_lowercase(){
  string line = "Een CamelCapped Zin.";
  string res = lowercase( line );
  assertEqual( res, "een camelcapped zin." );
}

void test_pretty_print(){
  stringstream out;
  set<string> s = {"jan","piet","klaas"};
  out << s << endl;
  assertEqual( out.str(), "{jan,klaas,piet}\n" );
  out.str("");
  out.clear();
  vector<string> v = {"jan","piet","klaas"};
  out << v << endl;
  assertEqual( out.str(), "[jan,piet,klaas]\n" );
  out.str("");
  out.clear();
  list<int> l = {5,34,3};
  out << l << endl;
  assertEqual( out.str(), "[5,34,3]\n" );
  out.str("");
  out.clear();
  multiset<string> ms = {"jan","jan","klaas"};
  out << ms << endl;
  assertEqual( out.str(), "{jan,jan,klaas}\n" );
  out.str("");
  out.clear();
  map<string,int> m = { {"jan",3},{"piet",55}, {"klaas",5} };
  out << m << endl;
  assertEqual( out.str(), "{<jan,3>,<klaas,5>,<piet,55>}\n" );
  out.str("");
  out.clear();
  multimap<string,int> mm = { {"jan",3},{"piet",55}, {"jan",5} };
  out << mm << endl;
  assertEqual( out.str(), "{<jan,3>,<jan,5>,<piet,55>}\n" );
  out.str("");
  out.clear();
  vector<pair<string,int>> pv = { {"marie",35},{"piet",34}, {"klaas",5} };
  out << pv << endl;
  assertEqual( out.str(), "[<marie,35>,<piet,34>,<klaas,5>]\n" );
}

void test_unicodehash(){
  Hash::UnicodeHash uh;
  size_t index = uh.hash( "appel" );
  assertEqual( index, 1 );
  index = uh.hash( "peer" );
  assertEqual( index, 2 );
  index = uh.hash( "禁禂" );
  assertEqual( index, 3 );
  index = uh.hash( "peer" );
  assertEqual( index, 2 );
  UnicodeString greek1 = "ἀντιϰειμένου";
  UnicodeString greek2 = "ἀντιϰειμένου"; //different normalizations!
  assertFalse( greek1 == greek2 );
  index = uh.hash( greek1 );
  assertEqual( index, 4 );
  index = uh.hash( greek2 );
  assertEqual( index, 4 );
  assertEqual( uh.num_of_entries(), 4 );
  assertEqual( uh.reverse_lookup( 3 ), "禁禂" );
}

void test_base_dir(){
  assertEqual( TiCC::basename("/foo/bar" ), "bar" );
  assertEqual( TiCC::dirname("/foo/bar" ), "/foo" );
  assertEqual( TiCC::basename("foo/bar" ), "bar" );
  assertEqual( TiCC::dirname("foo/bar" ), "foo" );
  assertEqual( TiCC::basename("foobar" ), "foobar" );
  assertEqual( TiCC::dirname("foobar" ), "." );
  assertEqual( TiCC::basename("/" ), "" );
  assertEqual( TiCC::dirname("/" ), "" );
  assertEqual( TiCC::basename("." ), "." );
  assertEqual( TiCC::dirname("." ), "." );
}

void test_realpath(){
  assertEqual( realpath( "" ), "" );
}

void test_bz2compression( const string& path ){
  assertTrue( bz2Compress( path + "small.txt", "bzout.bz2" ) );
  assertTrue( bz2Decompress( "bzout.bz2", "bzout.txt" ) );
  string buffer;
  assertNoThrow( buffer = bz2ReadFile( "bzout.bz2" ) );
  assertEqual( buffer.substr(0,4), "This" );
  string cmd = "diff " + path + "small.txt bzout.txt";
  assertEqual( system( cmd.c_str() ), 0 );
  bool ok = false;
  assertNoThrow( ok = bz2WriteFile( "bzout.test.bz2", buffer ) );
  assertEqual( ok, true );
  cmd = "diff bzout.bz2 bzout.test.bz2";
  assertEqual( system( cmd.c_str() ), 0 );
}

void test_gzcompression( const string& path ){
  assertTrue( gzCompress( path + "small.txt", "gzout.gz" ) );
  assertTrue( gzDecompress( "gzout.gz", "gzout.txt" ) );
  string buffer;
  assertNoThrow( buffer = gzReadFile( "gzout.gz" ) );
  assertEqual( buffer.substr(0,4), "This" );
  string cmd = "diff " + path + "small.txt gzout.txt";
  assertEqual( system(cmd.c_str()), 0 );
  assertTrue( gzDecompress( path + "nasty.gz", "nasty.txt" ) );
  cmd = "diff " + path + "nasty.gz nasty.txt";
  assertEqual( system(cmd.c_str()), 0 );
  bool ok = false;
  assertNoThrow( ok = gzWriteFile( "gzout.test.gz", buffer ) );
  assertEqual( ok, true );
  cmd = "diff gzout.gz gzout.test.gz";
  assertEqual( system( cmd.c_str() ), 0 );
}

void test_fileutils( const string& path ){
  vector<string> res;
  assertNoThrow( res = searchFilesExt( path, ".txt", false ) );
  assertEqual( res.size(), 1 );
  assertNoThrow( res = searchFilesExt( path, ".txt" ) );
  assertEqual( res.size(), 2 );
  assertNoThrow( res = searchFilesExt( path+"small.txt", ".txt" ) );
  assertEqual( res.size(), 1 );
  assertNoThrow( res = searchFiles( path+"small.txt" ) );
  assertEqual( res.size(), 1 );
  assertTrue( res[0] == path+"small.txt" );
  erase( "/tmp/test/silly/files/file" );
  assertTrue( createPath( "/tmp/test/silly/files/file" ) );
  erase( "/tmp/test/silly/" );
  assertTrue( createPath( "/tmp/test/silly/files/path/" ) );
  assertTrue( createPath( "/tmp/test/silly/files/path/raar" ) );
  assertFalse( createPath( "/tmp/test/silly/files/path/raar/sub" ) );

  assertNoThrow( res = searchFilesMatch( path, "*.txt", false ) );
  // non recursive. should match small.txt
  assertEqual( res.size(), 1 );
  assertNoThrow( res = searchFilesMatch( path, "*.txt" ) );
  // recursive should match small.txt and sub1/sub.txt
  assertEqual( res.size(), 2 );
  assertNoThrow( res = searchFilesMatch( path, "small" ) );
  // should match small.txt
  assertEqual( res.size(), 1 );
  assertNoThrow( res = searchFilesMatch( path, "s*[lb].txt" ) );
  // should match small.txt and sub1/sub.txt
  assertEqual( res.size(), 2 );
  string fn;
  {
    tmp_stream ts( "runtest1" );
    fn = ts.tmp_name();
    ofstream& os = ts.os();
    os << "TEST" << endl;
    ts.close();
    ifstream is( fn );
    string line;
    getline( is, line );
    assertEqual( line, "TEST" );
  }
  assertFalse( isFile( fn ) );
  {
    tmp_stream ts( "runtest2", "/var/tmp", true );
    fn = ts.tmp_name();
    assertTrue( isFile( fn ) );
    ofstream& os = ts.os();
    os << "TEST" << endl;
    ts.close();
    assertTrue( isFile( fn ) );
    ifstream is( fn );
    string line;
    getline( is, line );
    assertEqual( line, "TEST" );
    assertTrue( isFile( fn ) );
  }
  assertTrue( isFile( fn ) );
  assertNoThrow( erase( fn ) );
  assertFalse( isFile( fn ) );
  assertFalse( isFile( "/root/.config/ucto/tokconfig.nld" ) );
  erase( "/root/.config/ucto/tokconfig.nld" );
  assertTrue( isDir( "/" ) );
  assertFalse( isWritableDir( "/boot" ) );
}

void test_configuration( const string& path ){
  Configuration c;
  assertTrue( c.fill( path + "testconfig.cfg" ) );
  assertTrue( c.hasSection("test") );
  string att = c.lookUp( "jan" );
  assertEqual( att, "gek" );
  att = c.lookUp( "piet" );
  assertEqual( att, "" );
  att = c.lookUp( "piet", "test" );
  assertEqual( att, "ook gek" );
  att = c.lookUp( "kees", "test" );
  assertEqual( att, "een jongen" );
  att = c.lookUp( "klara", "test" );
  assertEqual( att, "speciaal=raar" );
  string oud = c.setatt( "jan", "normaal" );
  assertEqual( oud, "gek" );
  c.setatt( "pief", "paf", "poef" );
  c.clearatt( "piet", "test" );
  att = c.lookUp( "Truus", "test" );
  assertEqual( att, "met een \n er in en een \r!" );
  assertNoThrow( c.create_configfile( "/tmp/test.cfg" ) );
  Configuration c2;
  assertTrue( c2.fill( "/tmp/test.cfg" ) );
  att = c2.lookUp( "jan" );
  assertEqual( att, "normaal" );
  att = c2.lookUp( "klara", "test" );
  assertEqual( att, "speciaal=raar" );
  att = c2.lookUp( "kees", "test" );
  assertEqual( att, "een jongen" );
  att = c2.lookUp( "pief", "poef" );
  assertEqual( att, "paf" );
  att = c2.lookUp( "piet", "test" );
  assertEqual( att, "" );
  att = c.lookUp( "Truus", "test" );
  assertEqual( att, "met een \n er in en een \r!" );
  Configuration c3;
  assertTrue( c3.fill( path + "testconfig.cfg" ) );
  att = c3.lookUp( "jan" );
  assertEqual( att, "gek" );
  att = c3.lookUp( "pief", "poef" );
  assertEqual( att, "" );
  att = c3.lookUp( "piet", "test" );
  assertEqual( att, "ook gek" );
  c3.merge( c2 ); //default is NOT to override what is already there
  att = c3.lookUp( "jan" );
  assertEqual( att, "gek" ); // not overridden ?
  att = c3.lookUp( "pief", "poef" );
  assertEqual( att, "paf" );
  att = c3.lookUp( "piet", "test" ); // empty in c2, not changed in c3
  assertEqual( att, "ook gek" );
  c3.merge( c2, true ); // Now override
  att = c3.lookUp( "jan" );
  assertEqual( att, "normaal" ); // is overridden now?
  att = c3.lookUp( "piet", "test" ); // empty in c2, still not changed in c3
  assertEqual( att, "ook gek" );
  c3.setatt( "mik", "mak", "extra" );
  c3.setatt( "pim", "pam", "extra" );
  c3.setatt( "tip", "top", "extra" );
  assertEqual( c3.hasSection( "extra" ), true );
  Configuration::ssMap extra = c3.lookUpAll( "extra" );
  assertEqual( extra.size(), 3 );
  c3.erasesection( "extra" );
  assertEqual( c3.hasSection( "extra" ), false );
}

void test_logstream( const string& path ){
  ofstream uit( "/tmp/testls.1" );
  LogStream ls( uit );
  ls.set_stamp( NoStamp );
  *Log( ls ) << "test 1 level=" << ls.get_level() << " threshold="
	     << ls.get_threshold() << endl;
  *Dbg( ls ) << "debug 1" << endl;
  *xDbg( ls ) << "x_debug 1" << endl;
  *xxDbg( ls ) << "xx_debug 1" << endl;
  ls.set_level( LogSilent );
  *Log( ls ) << "test 2 level=" << ls.get_level() << " threshold="
	     << ls.get_threshold() << endl;
  *Dbg( ls ) << "debug 2" << endl;
  *xDbg( ls ) << "x_debug 2" << endl;
  *xxDbg( ls ) << "xx_debug 2" << endl;
  ls.set_level( LogDebug );
  *Log( ls ) << "test 3 level=" << ls.get_level() << " threshold="
	     << ls.get_threshold() << endl;
  *Dbg( ls ) << "debug 3" << endl;
  *xDbg( ls ) << "x_debug 3" << endl;
  *xxDbg( ls ) << "xx_debug 3" << endl;
  ls.set_level( LogExtreme );
  *Log( ls ) << "test 4 level=" << ls.get_level() << " threshold="
	     << ls.get_threshold() << endl;
  *Dbg( ls ) << "debug 4" << endl;
  *xDbg( ls ) << "x_debug 4" << endl;
  *xxDbg( ls ) << "xx_debug 4" << endl;
  ls.set_level( LogHeavy );
  *Log( ls ) << "test 5 level=" << ls.get_level() << " threshold="
	     << ls.get_threshold() << endl;
  ls.add_message( "AHA:" );
  ls.set_stamp( StampMessage );
  *Dbg( ls ) << "debug 5" << endl;
  *xDbg( ls ) << "x_debug 5" << endl;
  *xxDbg( ls ) << "xx_debug 5" << endl;
  string cmd = "diff /tmp/testls.1 " + path + "testls.1.ok";
  assertEqual( system( cmd.c_str() ), 0 );
}

void test_unicode( const string& path ){
  UChar32 uc0 = L'私';
  UnicodeString u1 = uc0;
  UChar32 uc1 = U'\U00007981';
  UChar32 uc2 = U'\U00007982';
  UnicodeString u2 = uc1;
  u2 += UnicodeString( uc2 );
  string s1 = UnicodeToUTF8( u1 );
  assertEqual( s1 , "私" );
  string s2 = UnicodeToUTF8( u2 );
  assertEqual( s2 , "禁禂" );
  ifstream in( path + "utf16bom.nl" );
  string line;
  getline( in, line );
  assertFalse( line == "Hier staat een BOM voor. æ en ™ om te testen." );
  UnicodeString u3 = UnicodeFromEnc( line, "UTF16" );
  string s3 = UnicodeToUTF8(  u3 );
  assertEqual( s3, "Hier staat een BOM voor. æ en ™ om te testen." );
  UnicodeString greek1 = "ἀντιϰειμένου";
  UnicodeString greek2 = "ἀντιϰειμένου";
  assertFalse( greek1 == greek2 ); // different normalizations!
  UnicodeNormalizer N1;
  UnicodeString ng11 = N1.normalize( greek1 );
  UnicodeString ng12 = N1.normalize( greek2 );
  assertEqual( UnicodeToUTF8(ng11), UnicodeToUTF8(ng12) );
  assertEqual( UnicodeToUTF8(ng11,"NFD"), UnicodeToUTF8(ng12,"NFD") );
  UnicodeNormalizer N2("NFD");
  UnicodeString ng21 = N2.normalize( greek1 );
  UnicodeString ng22 = N2.normalize( greek2 );
  assertEqual( UnicodeToUTF8(ng21), UnicodeToUTF8(ng22) );
  string mode="NFKD";
  UnicodeNormalizer N3(mode);
  UnicodeString ng31 = N3.normalize( greek1 );
  UnicodeString ng32 = N3.normalize( greek2 );
  assertEqual( UnicodeToUTF8(ng31), UnicodeToUTF8(ng32) );
  string utf8_1 = "ἀντιϰειμένου";
  string utf8_2 = "ἀντικειμένου";
  assertEqual( TiCC::utf8_uppercase( utf8_1 ), "ἈΝΤΙΚΕΙΜΈΝΟΥ" );
  assertEqual( TiCC::utf8_lowercase( "ἈΝΤΙΚΕΙΜΈΝΟΥ" ), utf8_2 );
  assertEqual( TiCC::utf8_uppercase( "æ en ß en œ" ), "Æ EN SS EN Œ" );
}

void test_unicode_trim(){
  UnicodeString tr1 = "dit is een test";
  UnicodeString tr2 = "\t  dit is een test \r ";
  UnicodeString tr3 = "    \ndit is een test";
  assertEqual( utrim(tr1), "dit is een test" );
  assertEqual( utrim(tr2), "dit is een test" );
  assertEqual( utrim(tr3), "dit is een test" );
  assertEqual( ltrim(tr1), "dit is een test" );
  assertEqual( ltrim(tr2), "dit is een test \r " );
  assertEqual( ltrim(tr3), "dit is een test" );
  assertEqual( rtrim(tr1), "dit is een test" );
  assertEqual( rtrim(tr2), "\t  dit is een test" );
  assertEqual( rtrim(tr3), "    \ndit is een test" );
  assertEqual( utrim(tr2, " \n\r"), "\t  dit is een test" );
  assertEqual( utrim(tr2, " \t"), "dit is een test \r" );
}

void test_unicode_split(){
  string line8 = "De kat krabt de krullen\n van de   trap.";
  UnicodeString line = TiCC::UnicodeFromUTF8( line8 );
  vector<UnicodeString> res = split_at( line, "de" );
  assertEqual( res.size(), 3 );
  assertEqual( TiCC::UnicodeToUTF8(res[0]), "De kat krabt " );
  assertEqual( TiCC::UnicodeToUTF8(res[1]), " krullen\n van " );
  assertEqual( TiCC::UnicodeToUTF8(res[2]), "   trap." );
  res = split( line, 3 );
  assertEqual( res.size(), 3 );
  assertEqual( res[1], "kat" );
  assertEqual( res[2], "krabt de krullen\n van de   trap." );
  res = split( line, 24 );
  assertEqual( res.size(), 8 );
  assertEqual( res[1], "kat" );
  assertEqual( res[2], "krabt" );
  assertEqual( res[4], "krullen" );
  assertEqual( res[5], "van" );
  UnicodeString vies = "em—dash, en–dash, bar―, bar―――, 3em⸻dash, FullWidth－HyphenMinus,";
  res = split_at( vies, "," );
  assertEqual( res.size(), 6 );
  assertEqual( res[5], " FullWidth－HyphenMinus" );
  UnicodeString seps =  "—–―⸻－";
  res = split_at_first_of( vies, seps );
  assertEqual( res.size(), 7 );
  assertEqual( res[0], "em" );
  assertEqual( res[2], "dash, bar" );
  assertEqual( res[4], ", 3em" );
  assertEqual( res[6], "HyphenMinus," );
}

void test_unicode_split_exact(){
  UnicodeString vies = "em—dash, en–dash,, bar―, bar―――, 3em⸻dash, FullWidth－HyphenMinus,";
  vector<UnicodeString> res = split_exact_at( vies, "," );
  assertEqual( res.size(), 8 );
  assertEqual( res[6], " FullWidth－HyphenMinus" );
  UnicodeString seps =  "—–―⸻－";
  res = split_exact_at_first_of( vies, seps );
  assertEqual( res.size(), 9 );
  assertEqual( res[0], "em" );
  assertEqual( res[1], "dash, en" );
  assertEqual( res[2], "dash,, bar" );
  assertEqual( res[3], ", bar" );
  assertEqual( res[4], "" );
  assertEqual( res[5], "" );
  assertEqual( res[6], ", 3em" );
  assertEqual( res[7], "dash, FullWidth" );
  assertEqual( res[8], "HyphenMinus," );
}

void test_unicode_split_at_exact(){
  UnicodeString line = "1/2//4////8//10";
  vector<UnicodeString> res = split_exact_at( line, "/" );
  assertEqual( res.size(), 10 );
  assertEqual( res[5], "" );
}

void test_unicode_split_at(){
  UnicodeString line = "Derarekatrarekrabtrarederarekrullen\nrarevanrarederaretrap.";
  vector<UnicodeString> res = split_at( line, "rare" );
  assertEqual( res.size(), 8 );
  assertEqual( res[5], "van" );
  vector<UnicodeString> res2 = split_at( line, "rare", 4 );
  assertEqual( res2.size(), 4 );
  assertEqual( res2[2], "krabt" );
  assertEqual( res2[3], "derarekrullen\nrarevanrarederaretrap." );
}

void test_unicode_split_at_first(){
  UnicodeString line = "De.kat,krabt:de;krullen?van.,;.;de!trap.";
  vector<UnicodeString> res = split_at_first_of( line, ".,?!:;" );
  assertEqual( res.size(), 8 );
  assertEqual( res[5], "van" );
  vector<UnicodeString> res2 = split_at_first_of( line, ".,?!:;", 7 );
  assertEqual( res2.size(), 7 );
  assertEqual( res2[4], "krullen" );
  assertEqual( res2[6], ",;.;de!trap." );
}

void test_unicode_split_at_first_exact(){
  UnicodeString line = "De.kat,krabt:de;krullen?van.,;.;de!trap.";
  vector<UnicodeString> res = split_exact_at_first_of( line, ".,?!:;" );
  assertEqual( res.size(), 13 );
  //  cerr << "after split: " << res << endl;
  assertEqual( res[5], "van" );
  assertEqual( res[9], "" );
}

void test_unicode_regex( ){
  string pattern1 = "^(\\p{Lu}{1,2}\\.{1,2}(\\p{Lu}{1,2}\\.{1,2})*)(\\p{Lu}{0,2})$";
  UnicodeRegexMatcher test1( UnicodeFromUTF8(pattern1), "test1" );
  UnicodeString pre, post;
  UnicodeString us = "A.N.W.B.";
  assertTrue( test1.match_all( us, pre, post ) );
  us = "A.N.W..B";
  assertTrue( test1.match_all( us, pre, post ) );
  us = "A.NON.W.B.";
  assertFalse( test1.match_all( us, pre, post ) );
  string pattern2 = "(?:de|het|een)_(\\p{Lu}+)(?:-(?:\\p{L}*)|\\Z)";
  UnicodeRegexMatcher test2( UnicodeFromUTF8(pattern2), "test2" );
  us = "een_CDA-minister";
  assertTrue( test2.match_all( us, pre, post ) );
  string result = TiCC::UnicodeToUTF8( test2.get_match( 0 ) );
  assertEqual( result, "CDA" );
  us = "de_VVD";
  assertTrue( test2.match_all( us, pre, post ) );
  result = TiCC::UnicodeToUTF8( test2.get_match( 0 ) );
  assertEqual( result, "VVD" );
}

void test_unicode_filters( const string& path ){
  UniFilter filt;
  assertNoThrow( filt.init( "‘ > \\' ; ’ > \\' ;  \\` > \\' ; ´ > \\' ;",
			    "quote_filter" ) );
  UnicodeString vies = "`vies´ en ‘smerig’ en `apart´";
  UnicodeString schoon = filt.filter( vies );
  assertEqual( schoon, "\'vies\' en \'smerig\' en \'apart\'" );
  UniFilter filt2;
  assertNoThrow( filt2.fill( path + "quotes.filter") );
  schoon = filt.filter( vies );
  assertEqual( schoon, "\'vies\' en \'smerig\' en \'apart\'" );
  UniFilter filt3;
  assertNoThrow( filt3.fill( path + "quotes.old.filter") );
  schoon = filt.filter( vies );
  assertEqual( schoon, "\'vies\' en \'smerig\' en \'apart\'" );
  UniFilter filt4;
  assertNoThrow( filt4.init( "‘ > \\' ;", "quote_filter" ) );
  assertNoThrow( filt4.add( string("’ \' ") ) );
  assertNoThrow( filt4.add( string("` \'") ) );
  assertNoThrow( filt4.add( string("´ \' ") ) );
  schoon = filt.filter( vies );
  assertEqual( schoon, "\'vies\' en \'smerig\' en \'apart\'" );
  UniFilter filt5;
  assertNoThrow( filt5.init( "[:Hyphen:]+ > '-'; [:Dash:]+ > '-';",
			     "hypen_filter" ) );
  vies = "em—dash, en–dash, bar―, bar―――, 3em⸻dash, FullWidth－HyphenMinus,"
    " 3FullWidth－－－HyphenMinus, vertical︱Emdash, hyp-en, 2hyp--ens";
  schoon = filt5.filter( vies );
  assertEqual( schoon, "em-dash, en-dash, bar-, bar-, 3em-dash, "
	       "FullWidth-HyphenMinus, 3FullWidth-HyphenMinus, vertical-Emdash,"
	       " hyp-en, 2hyp-ens" );
  UniFilter filt6;
  assertNoThrow( filt6.init( "^[:White_Space:]+ > ; [:White_Space:]+ > ' ';", "sep_filter" ) );
  vies = " \t\t  Jan    en    Kees, \tKlaas\t \ten    Mies";
  schoon = filt6.filter( vies );
  assertEqual( schoon, "Jan en Kees, Klaas en Mies" );
  assertEqual( filter_diacritics( "een appél is geen appèl" ), "een appel is geen appel" );
  assertEqual( filter_diacritics( "de reeën zijn reeël" ), "de reeen zijn reeel" );
}

void test_conversion(){
  int i = 8;
  double d = 3.14;
  string result;
  assertNoThrow( result = toString( i ) );
  assertEqual( result, "8" );
  int ii;
  assertNoThrow( ii = stringTo<int>( result ) );
  assertEqual( i, ii );
  assertNoThrow( result = toString( d ) );
  assertEqual( result, "3.14" );
  double dd;
  assertNoThrow( dd = stringTo<double>( result ) );
  assertEqual( d, dd );
  string fout = "appeltaart";
  assertThrow( dd = stringTo<double>( fout ), runtime_error );
  string error = lastError();
  assertEqual( error, "conversion from string 'appeltaart' to type:d failed" );
  fout = "a2.718q";
  assertNoThrow( dd = stringTo<double>( fout ) );
  if ( hasThrown() ){
    decrementError();
    string new_error = lastError();
    assertEqual( new_error,
		 "conversion from string 'a2.718q' to type:d failed" );
  }
}

bool faal(){
  throw runtime_error( "I failed it" );
}

void test_assert() {
  assertTrue( faal() );
}

void test_json() {
  using namespace nlohmann;
  string J = "{\"label\":\"Dit is een test.\"}";
  json parsed = json::parse( J );
  assertEqual( parsed["label"] , "Dit is een test." )
}

enum flags { No = 0, One = 1, Two= 2, Four = 4};

enum class class_flags { nope = 0, ok = 1, warning = 1<<1, error = 1<<2 };

DEFINE_ENUM_FLAG_OPERATORS(flags);
DEFINE_ENUM_FLAG_OPERATORS(class_flags);

std::ostream& operator<<( std::ostream& os, const flags& f ){
  os << int(f);
  return os;
}

std::ostream& operator<<( std::ostream& os, const class_flags& f ){
  os << int(f);
  return os;
}

void test_enum_flags() {
  {
    flags f = flags::Two|flags::Four;
    cerr << f << endl;
    cerr << (f & (flags::Two|flags::Four) ) << endl;
    cerr << (f & flags::Two) << endl;
    // cppcheck-suppress knownConditionTrueFalse
    assertTrue( f == 6 );
    assertTrue( f % (flags::Two|flags::Four) );
    f = ~f;
    // cppcheck-suppress knownConditionTrueFalse
    assertEqual( f, -7 );
    f &= flags::One;
    std::stringstream ss;
    ss << f;
    assertEqual( ss.str(), "1" );
    assertEqual( f, 1 );
  }
  {
    //  DEFINE_ENUM_FLAGS works for both 'enum' and 'enum class'
    // BUT: the assertion macro's have a problem with the latter
    //      needs work. Now we need an explicit cast
    class_flags f = class_flags::warning|class_flags::error;
    std::stringstream ss;
    ss << f;
    assertEqual( ss.str(), "6" );
    // cppcheck-suppress knownConditionTrueFalse
    assertTrue( (int)f == 6 );
    assertTrue( f%(class_flags::warning|class_flags::error) );
    assertTrue( f%class_flags::warning );
    assertTrue( f%class_flags::error );
    assertFalse( f%class_flags::ok );
    f = ~f;
    assertEqual( int(f), -7 );
    f &= class_flags::ok;
    assertEqual( (int)f, 1 );
    assertTrue( (f%class_flags::ok) );
    assertFalse( (f%class_flags::warning) );
  }
}

#include <type_traits>

#define ADD_FUN_CHECK( FUN ) \
template<typename, typename T>			\
struct has_##FUN {			\
  static_assert(						\
		std::integral_constant<T, false>::value,		\
		"Second template parameter needs to be of function type." ); \
};									\
									\
template<typename C, typename Ret, typename... Args> \
struct has_##FUN<C, Ret(Args...)> { \
private: \
  template<typename T> \
  static constexpr auto check(T*) \
    -> typename			  \
    std::is_same<							\
    decltype( std::declval<T>().FUN( std::declval<Args>()... ) ), \
    Ret \
  >::type; \
						\
  template<typename>				\
  static constexpr std::false_type check(...);	\
						\
  using type = decltype(check<C>(0));		\
						\
public:						\
 static constexpr bool value = type::value;	\
};

ADD_FUN_CHECK( string_fun )
ADD_FUN_CHECK( unistring_fun )
ADD_FUN_CHECK( int_fun )

void test_templates(){

  struct X {
    bool string_fun( const std::string& ){
      return true;
    }
  };
  struct Y:X {
    bool unistring_fun( const icu::UnicodeString& ){
      return true;
    }
    int int_fun( const int& i, int j ){
      return i+j;
    }
  };

  bool test_val = has_string_fun<Y,bool(const std::string&)>::value;
  assertEqual( test_val, true );
  test_val = has_unistring_fun<Y,bool(const icu::UnicodeString&)>::value;
  assertEqual( test_val, true );
  test_val = has_string_fun<X,bool(const std::string&)>::value;
  assertEqual( test_val, true );
  test_val = has_unistring_fun<X,bool(const icu::UnicodeString&)>::value;
  assertEqual( test_val, false );
  test_val = has_string_fun<X,bool(int,double)>::value;
  assertEqual( test_val, false );
  test_val = has_int_fun<Y,int(int,int)>::value;
  assertEqual( test_val, true );
  test_val = has_int_fun<Y,int(const int,int)>::value;
  assertEqual( test_val, true );
  test_val = has_int_fun<Y,int(const int&,int)>::value;
  assertEqual( test_val, true );
}

void test_ncname(){
  assertFalse( isNCName("123") );
  assertTrue( isNCName("_123") );
  assertEqual( create_NCName( "12?name" ), "name" );
  assertEqual( create_NCName("aap!noot"), "aapnoot" );
  assertEqual( create_NCName("A#12!3"), "A123" );
  assertEqual( create_NCName(".-_!A#12!3"), "_A123" );
  assertEqual( create_NCName("_appel-taart.met slagroom_"), "_appel-taart.met_slagroom_" );
}

int main( const int argc, const char* argv[] ){
  cerr << BuildInfo() << endl;
  Timer t1;
  Timer t2;
  t1.start();
  t2.start();
  test_opts_basic();
  CL_Options opts1;
  opts1.add_short_options( "t:qf:d:" );
  opts1.add_long_options( "test:,raar" );
  opts1.init( argc, argv );
  test_opts( opts1 );
  CL_Options opts2;
  opts2.allow_args( "t:qf:d:", "test:,raar" );
  opts2.parse_args( "-ffalse +t true --test=test -d iets -q --raar blaat arg1 arg2 --SetCommandLineDebug" );
  test_opts( opts2 );
  test_subtests_fail();
  test_subtests_ok();
  test_throw();
  test_nothrow();
  test_trim();
  test_trim_front();
  test_trim_back();
  test_pad();
  test_match_front();
  test_match_back();
  test_format_non_printable();
  test_split();
  test_split_exact();
  test_split_at();
  test_split_at_exact();
  test_split_at_first();
  test_split_at_first_exact();
  test_to_upper();
  test_to_lower();
  test_uppercase();
  test_lowercase();
  test_unicodehash();
  test_realpath();
  test_ncname();
  string testdir;
  bool dummy;
  opts1.is_present( 'd', testdir, dummy );
  test_bz2compression( testdir );
  test_gzcompression( testdir );
  test_base_dir();
  test_fileutils( testdir );
  test_configuration( testdir );
  test_pretty_print();
  test_logstream( testdir );
  test_unicode( testdir );
  test_unicode_split();
  test_unicode_split_exact();
  test_unicode_split_at();
  test_unicode_split_at_exact();
  test_unicode_split_at_first();
  test_unicode_split_at_first_exact();
  test_unicode_trim();
  test_unicode_regex();
  test_unicode_filters( testdir );
  test_conversion();
  test_assert();
  test_json();
  test_enum_flags();
  test_templates();
  t1.stop();
  t2.stop();
  cerr << t1 << endl;
  cerr << t2 << endl;
  cerr << t1 + t2 << endl;
  summarize_tests(5);
}
