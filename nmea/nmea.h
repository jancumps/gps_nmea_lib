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
#include <vector>
#include <string_view>
#include <chrono>

namespace nmea {

typedef std::chrono::hh_mm_ss<std::chrono::duration<int, std::ratio<1, 1000>>> time_t;

class nmea {
public:
	enum talker_id {
		gps,     			// If system works in GPS only mode
		glonass, 			// If system works in GLONASS only mode
		galileo, 			// If system works in GALILEO only mode
		beidou,  			// If system works in BEIDOU only mode
		qzss,    			// If system works in QZSS only mode
		multiconstellation 	// If system works in multi-constellation mode
	};

	enum dir {
		n,
		s,
		e,
		w
	};

	enum qual : unsigned int {
		q0 = 0,
		q1 = 1,
		q2 = 2,
		q6 = 6
	};

	nmea() = delete; // prevent  creation of objects of this utility class

	static talker_id get_talker_id(const std::string_view& sv);
	static talker_id get_system_id(const std::string_view& sv);
	static float get_coord(const std::string_view& sv);
	static dir get_dir(const std::string_view& sv);
	static void get_time(const std::string_view& sv, time_t& t);
	static void get_date(const std::string_view& sv, std::chrono::year_month_day& d);
	static bool get_valid(const std::string_view& sv);
	static qual get_qual(const std::string_view& sv);
};

class gll {
public:
	static bool from_data(const std::string& data, gll& gll);
	nmea::talker_id source;
	float lat;
    float lon;
    time_t t;
    bool valid;
};

class gga {
public:
	static bool from_data(const std::string& data, gga& gga);
	nmea::talker_id source;
    float lat;
    float lon;
    time_t t;
    unsigned int sats;
    unsigned int qual;
};

typedef std::array<unsigned int, 12> gsa_sat_array;

class gsa {
public:
	static bool from_data(const std::string& data, gsa& gsa);
	nmea::talker_id source;
	nmea::talker_id system_id;
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
	nmea::talker_id source;
	gsv_sat_array sats;
};

class rmc {
public:
	static bool from_data(const std::string& data, rmc& rmc);
	nmea::talker_id source;
	float lat;
    float lon;
    float speed;
    time_t t;
    std::chrono::year_month_day d;
    bool valid;
};

} // namespace nmea

#endif /* NMEA_H_ */
