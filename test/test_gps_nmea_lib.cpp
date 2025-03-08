#include <gtest/gtest.h>
#include <string>
import nmea;

// generic template for NMEA parser unit tests
// base class for all test fixtures, parameterized or not.
template <class T>
class nmeaTest : public testing::Test {
protected:
    nmeaTest() {}
    T o;
};

// ==================================================================================
// template for parameterised parser tests
// generic fixture that allows to test any nmea parser class' parser function
// with multiple data inputs
template <class T>
class nmeaParsers : public nmeaTest<T>, public testing::WithParamInterface<std::tuple<std::string, bool>> {
protected:
    nmeaParsers() {}
};

class nmea_gllParser : public nmeaParsers<nmea::gll> {};

TEST_P(nmea_gllParser, gllparsetest) {
    bool expected = std::get<1>(GetParam());
    std::string s = std::get<0>(GetParam());
    ASSERT_EQ(expected, o.from_data(s, o));
}

INSTANTIATE_TEST_SUITE_P(
    ,
    nmea_gllParser,
    ::testing::Values(
        std::make_tuple("$XXXXXXXX", false), // invalid payload
        std::make_tuple("", false), // empty payload
        std::make_tuple("$GPGLL,5051.83778,N,00422.55809,S,185427.150,A,N*4F", true),
        std::make_tuple("$GPGLL,5051.83778,N,00422.55809,S,185427.150,V,N*4F", true))
);

class nmea_ggaParser : public nmeaParsers<nmea::gga> {};

TEST_P(nmea_ggaParser, ggaparsetest) {
    bool expected = std::get<1>(GetParam());
    std::string s = std::get<0>(GetParam());
    ASSERT_EQ(expected, o.from_data(s, o));
}

INSTANTIATE_TEST_SUITE_P(
    ,
    nmea_ggaParser,
    ::testing::Values(
        std::make_tuple("$XXXXXXXX", false), // invalid payload
        std::make_tuple("", false), // empty payload
        std::make_tuple("$GPGGA,191237.000,5051.78066,N,00422.57079,E,1,05,3.7,027.26,M,47.3,M,,*65", true))
);

class nmea_gsaParser : public nmeaParsers<nmea::gsa> {};

TEST_P(nmea_gsaParser, gsaparsetest) {
    bool expected = std::get<1>(GetParam());
    std::string s = std::get<0>(GetParam());
    ASSERT_EQ(expected, o.from_data(s, o));
}

INSTANTIATE_TEST_SUITE_P(
    ,
    nmea_gsaParser,
    ::testing::Values(
        std::make_tuple("$XXXXXXXX", false), // invalid payload
        std::make_tuple("", false), // empty payload
        std::make_tuple("$GNGSA,A,3,15,18,,,,,,,,,,,4.7,3.7,2.9*2D", true),
        std::make_tuple("$GNGSA,A,3,73,65,81,,,,,,,,,,4.7,3.7,2.9*2E", true))
);

// ==================================================================================

// unit test for gll parser
class nmea_gll : public nmeaTest<nmea::gll> {
protected:
    nmea_gll() : parse_ok(false) {}
    void SetUp() override {
        parse_ok = o.from_data("$GPGLL,5051.83778,N,00422.55809,S,185427.150,A,N*4F", o);
    }
    bool parse_ok;
};

TEST_F(nmea_gll, parse) {
    EXPECT_TRUE(parse_ok) << "parse failed";
}
    
TEST_F(nmea_gll, source) {
    EXPECT_TRUE(o.source == nmea::talker_id::gps);
}
    
TEST_F(nmea_gll, lat) {
    EXPECT_FLOAT_EQ(o.lat, 50.8639641);
}
    
TEST_F(nmea_gll, lon) {
    EXPECT_FLOAT_EQ(o.lon, 4.37596798);
}
 
TEST_F(nmea_gll, time) {
    EXPECT_EQ((int)(o.t.hours().count()), 18) << "hours wrong. expected: 18. got: " << (int)(o.t.hours().count());
    EXPECT_EQ((int)(o.t.minutes().count()), 54) << "minutes wrong";
    EXPECT_EQ((int)(o.t.seconds().count()), 27) << "seconds wrong";
    EXPECT_EQ((int)(o.t.subseconds().count()), 150) << "subseconds wrong";
}

TEST_F(nmea_gll, valid) {
    EXPECT_TRUE(o.valid);
}
    
// unit test for gga parser
class nmea_gga : public nmeaTest<nmea::gga> {
protected:
    nmea_gga() : parse_ok(false) {}
    void SetUp() override {
        parse_ok = o.from_data("$GPGGA,191237.000,5051.78066,N,00422.57079,E,1,05,3.7,027.26,M,47.3,M,,*65", o);
    }
    bool parse_ok;
};
    
TEST_F(nmea_gga, parse) {
    EXPECT_TRUE(parse_ok) << "parse failed";
}

TEST_F(nmea_gga, source) {
    EXPECT_TRUE(o.source == nmea::talker_id::gps);
}
    
TEST_F(nmea_gga, lat) {
    EXPECT_FLOAT_EQ(o.lat, 50.86301);
}
    
TEST_F(nmea_gga, lon) {
    EXPECT_FLOAT_EQ(o.lon, 4.3761797);
}
 
TEST_F(nmea_gga, time) {
    EXPECT_EQ((int)(o.t.hours().count()), 19) << "hours wrong. expected: 18. got: " << (int)(o.t.hours().count());
    EXPECT_EQ((int)(o.t.minutes().count()), 12) << "minutes wrong";
    EXPECT_EQ((int)(o.t.seconds().count()), 37) << "seconds wrong";
    EXPECT_EQ((int)(o.t.subseconds().count()), 0) << "subseconds wrong";
}

TEST_F(nmea_gga, sats) {
    EXPECT_EQ(o.sats, 5);
}

TEST_F(nmea_gga, qual) {
    EXPECT_EQ(o.qual, nmea::quality::q1);
}

TEST_F(nmea_gga, alt) {
    EXPECT_FLOAT_EQ(o.alt, 27.2600002);
}

TEST_F(nmea_gga, geosep) {
    EXPECT_FLOAT_EQ(o.geosep, 47.2999992);
}