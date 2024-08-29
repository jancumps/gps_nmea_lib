/*
 * nmea.h
 *
 *  Created on: 29 jul. 2024
 *      Author: jancu
 */

#ifndef NMEA_H_
#define NMEA_H_

#include <string>
#include <array>
#include <string_view>
#include <chrono>

namespace nmea {

typedef std::chrono::hh_mm_ss<std::chrono::duration<long long, std::ratio<1, 1000>>> time_t;

enum class talker_id {
	notset,				// avoid that an initialised but unset variable assumes the first valid value
	gps,     			// If system works in GPS only mode
	glonass, 			// If system works in GLONASS only mode
	galileo, 			// If system works in GALILEO only mode
	beidou,  			// If system works in BEIDOU only mode
	qzss,    			// If system works in QZSS only mode
	multiconstellation 	// If system works in multi-constellation mode
};

enum class direction {
	n,
	s,
	e,
	w
};

enum class quality : unsigned int {
	q0 = 0,
	q1 = 1,
	q2 = 2,
	q6 = 6
};

class nmea {
public:
	nmea() = delete; // prevent  creation of objects of this utility class

	static talker_id talker(const std::string_view& sv);
	static talker_id system(const std::string_view& sv);
	static float coord(const unsigned int degrees_chars, const std::string_view& sv);
	static direction dir(const std::string_view& sv);
	static void time(const std::string_view& sv, time_t& t);
	static void date(const std::string_view& sv, std::chrono::year_month_day& d);
	static bool valid(const std::string_view& sv);
	static quality qual(const std::string_view& sv);
};

class gll {
public:
	static bool from_data(const std::string& data, gll& gll);
	talker_id source;
	float lat;
    float lon;
    time_t t;
    bool valid;
};

class gga {
public:
	static bool from_data(const std::string& data, gga& gga);
	talker_id source;
    float lat;
    float lon;
    time_t t;
    unsigned int sats;
    quality qual;
	float alt;
};

typedef std::array<unsigned int, 12> gsa_sat_array;

class gsa {
public:
	static bool from_data(const std::string& data, gsa& gsa);
	talker_id source;
	talker_id system_id;
	gsa_sat_array sats;
};

class gsv_sat {
public:
	unsigned int prn;
	unsigned int elev;
	unsigned int azim;
	unsigned int snr;
};

typedef std::array<gsv_sat, 4> gsv_sat_array;

class gsv {
public:
	static bool from_data(const std::string& data, gsv& gsv);
	talker_id source;
	gsv_sat_array sats;
};

class rmc {
public:
	static bool from_data(const std::string& data, rmc& rmc);
	talker_id source;
	float lat;
    float lon;
    float speed;
    time_t t;
    std::chrono::year_month_day d;
    bool valid;
};

} // namespace nmea

#endif /* NMEA_H_ */
