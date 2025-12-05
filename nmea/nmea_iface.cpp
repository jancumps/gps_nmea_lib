/*
 * nmea_iface.cpp
 *
 *  Created on: 29 jul. 2024
 *      Author: jancu
 */

module;
 
#include <string>
#include <array>
#include <string_view>
#include <chrono>
#include <concepts>
#include <type_traits>

export module nmea;

export namespace nmea {

using time_t = std::chrono::hh_mm_ss<std::chrono::duration<long long, std::ratio<1, 1000>>>;

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

template<typename T>
concept has_success_bool =
	requires(T t) {
	   	true; // { t.success; } -> std::same_as<bool>; // need to read up
	};

template <has_success_bool T> struct nmea_result {
    T result;
    constexpr explicit operator bool() const noexcept { return result.success; }
};

struct gll {
	using gll_result = nmea_result<gll>;
	static gll_result from_data(const std::string& data);
	talker_id source;
	float lat;
    float lon;
    time_t t;
    bool valid;
	bool success = false;
};

struct gga {
	using gga_result = nmea_result<gga>;
	static gga_result from_data(const std::string& data);
	talker_id source;
    float lat;
    float lon;
    time_t t;
    unsigned int sats;
    quality qual;
	float alt;
	float geosep;
	bool success = false;
};

using gsa_sat_array = std::array<unsigned int, 12>;

struct gsa {
	using gsa_result = nmea_result<gsa>;
	static gsa_result from_data(const std::string& data);
	talker_id source;
	talker_id system_id;
	gsa_sat_array sats;
	bool success = false;
};

struct gsv_sat {
	unsigned int prn;
	unsigned int elev;
	unsigned int azim;
	unsigned int snr;
};

using gsv_sat_array = std::array<gsv_sat, 4>;

struct gsv {
	using gsv_result = nmea_result<gsv>;
	static gsv_result from_data(const std::string& data);
	talker_id source;
	gsv_sat_array sats;
	bool success = false;
};

struct rmc {
	using rmc_result = nmea_result<rmc>;
	static rmc_result from_data(const std::string& data);
	talker_id source;
	float lat;
    float lon;
    float speed;
    time_t t;
    std::chrono::year_month_day d;
    bool valid;
	bool success = false;
};

} // namespace nmea