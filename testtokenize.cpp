/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


// The preprocessor that c++check uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "testsuite.h"
#define UNIT_TESTING        // Get access to "private" data in Tokenizer
#include "tokenize.h"

extern std::ostringstream errout;
class TestTokenizer : public TestFixture
{
public:
    TestTokenizer() : TestFixture("TestTokenizer")
    { }

private:

    void run()
    {
        TEST_CASE( multiline );
        TEST_CASE( longtok );

        TEST_CASE( inlineasm );

        TEST_CASE( dupfuncname );

        TEST_CASE( alwaysTrue );
        TEST_CASE( alwaysFalse );
    }


    bool cmptok(const char *expected[], const TOKEN *actual)
    {
        unsigned int i = 0;
        for (; expected[i] && actual; ++i, actual = actual->next)
        {
            if ( strcmp( expected[i], actual->str ) != 0)
                return false;
        }
        return (expected[i] == NULL && actual == NULL);
    }


    void multiline()
    {
        const char filedata[] = "#define str \"abc\" \\\n"
                                "            \"def\"\n";

        // tokenize..
        Tokenizer tokenizer( this );
        tokenizer.getFiles()->push_back( "test.cpp" );
        std::istringstream istr(filedata);
        tokenizer.TokenizeCode(istr, 0);

        // Expected result..
        const char *expected[] =
        {
            "def",
            "str",
            ";",
            0
        };

        // Compare..
        ASSERT_EQUALS( true, cmptok(expected, tokenizer.tokens()) );
    }


    void longtok()
    {
        std::string filedata(10000,'a');

        // tokenize..
        Tokenizer tokenizer( this );
        tokenizer.getFiles()->push_back( "test.cpp" );
        std::istringstream istr(filedata);
        tokenizer.TokenizeCode(istr, 0);

        // Expected result..
        ASSERT_EQUALS( std::string(10000,'a'), std::string(tokenizer.tokens()->str) );
    }


    void inlineasm()
    {
        const char filedata[] = "void foo()\n"
                                "{\n"
                                "    __asm\n"
                                "    {\n"
                                "        jmp $jump1\n"
                                "        $jump1:\n"
                                "    }\n"
                                "}\n";

        // tokenize..
        Tokenizer tokenizer( this );
        tokenizer.getFiles()->push_back( "test.cpp" );
        std::istringstream istr(filedata);
        tokenizer.TokenizeCode(istr, 0);

        // Expected result..
        const char *expected[] =
        {
            "void",
            "foo",
            "(",
            ")",
            "{",
            "}",
            0
        };

        // Compare..
        ASSERT_EQUALS( true, cmptok(expected, tokenizer.tokens()) );
    }


    void dupfuncname()
    {
        const char code[] = "void a()\n"
                            "{ }\n"
                            "void a(int i)\n"
                            "{ }\n"
                            "void b()\n"
                            "{ }\n";
        // tokenize..
        Tokenizer tokenizer( this );
        tokenizer.getFiles()->push_back( "test.cpp" );
        std::istringstream istr(code);
        tokenizer.TokenizeCode(istr, 0);

        tokenizer.FillFunctionList(0);

        ASSERT_EQUALS( 1, tokenizer.FunctionList.size() );
        ASSERT_EQUALS( std::string("b"), tokenizer.FunctionList[0]->str );
    }


    bool alwaysTrueCheck(const char condition[])
    {
        // tokenize..
        Tokenizer tokenizer( this );
        tokenizer.getFiles()->push_back( "test.cpp" );
        std::istringstream istr(condition);
        tokenizer.TokenizeCode(istr, 0);

        // check..
        return tokenizer.alwaysTrue( tokenizer.tokens() );
    }

    void alwaysTrue()
    {
        ASSERT_EQUALS(true, alwaysTrueCheck("(1)"));
        ASSERT_EQUALS(true, alwaysTrueCheck("(true)"));
        ASSERT_EQUALS(true, alwaysTrueCheck("(true || false)"));
        ASSERT_EQUALS(false, alwaysTrueCheck("(true && false)"));
    }

    bool alwaysFalseCheck(const char condition[])
    {
        // tokenize..
        Tokenizer tokenizer( this );
        tokenizer.getFiles()->push_back( "test.cpp" );
        std::istringstream istr(condition);
        tokenizer.TokenizeCode(istr, 0);

        // check..
        return tokenizer.alwaysFalse( tokenizer.tokens() );
    }

    void alwaysFalse()
    {
        ASSERT_EQUALS(true, alwaysFalseCheck("(0)"));
        ASSERT_EQUALS(true, alwaysFalseCheck("(false)"));
        ASSERT_EQUALS(true, alwaysFalseCheck("(false && true)"));
        ASSERT_EQUALS(false, alwaysFalseCheck("(false || true)"));
    }
};

REGISTER_TEST( TestTokenizer )
