/*
 * nmea.cpp
 *
 *  Created on: 29 jul. 2024
 *      Author: jancu
 */

module;

#include <cassert>
#include <ranges>
#include <cmath>
#include <chrono>

module nmea;

using std::operator""sv;
constexpr auto delim{","sv};

namespace nmea {

talker_id nmea::talker(const std::string_view& sv) {
	talker_id id = talker_id::gps;
	if (sv.starts_with("$GP")) {
		id = talker_id::gps;
	} else if (sv.starts_with("$GL")) {
		id = talker_id::glonass;
	} else if (sv.starts_with("$GA")) {
		id = talker_id::galileo;
	} else if (sv.starts_with("$BD")) {
		id = talker_id::beidou;
	} else if (sv.starts_with("$QZ")) {
		id = talker_id::qzss;
	} else if (sv.starts_with("$GN")) {
		id = talker_id::multiconstellation;
	}
	return id;
}

talker_id nmea::system(const std::string_view& sv) {
	talker_id id = talker_id::gps;
	if (sv.starts_with("1")) {
		id = talker_id::gps;
	} else if (sv.starts_with("2")) {
		id = talker_id::glonass;
	} else if (sv.starts_with("3")) {
		id = talker_id::galileo;
	} else if (sv.starts_with("4")) {
		id = talker_id::beidou;
	} else if (sv.starts_with("5")) {
		id = talker_id::qzss;
	}
	return id;
}

float nmea::coord(const unsigned int degrees_chars, const std::string_view& sv) {
	std::string s(sv);
	// a float (and a double) may not precisely represent the value in the message
	float coord = std::stof(s.substr(0, degrees_chars), nullptr);
	coord += std::stof(s.substr(degrees_chars), nullptr) / 60.0;
	return coord;
}

direction nmea::dir(const std::string_view& sv) {
	assert(sv.starts_with('N') ||
			sv.starts_with('S') ||
			sv.starts_with('E') ||
			sv.starts_with('W'));
	direction d = direction::n;
	if (sv.starts_with('N')) {
		d = direction::n;
	} else if (sv.starts_with('S')) {
		d = direction::s;
	} else if (sv.starts_with('E')) {
		d = direction::e;
	} else if (sv.starts_with('W')) {
		d = direction::w;
	}
	return d;
}

time_t nmea::time(const std::string_view& sv) {
	std::string s(sv);
	return time_t{
			std::chrono::hours(std::stoi(s.substr(0,  2))) +
			std::chrono::minutes(std::stoi(s.substr(2,  2))) +
		    std::chrono::seconds(std::stoi(s.substr(4,  2))) +
			std::chrono::milliseconds(std::stoi(s.substr(7)))
	};
}

std::chrono::year_month_day nmea::date(const std::string_view& sv) {
	std::string s(sv);
// NMEA returns two digits, and not all systems know what century we are in.
// Current solution: use the century defined in source code.
// alternative could be to convert to stream and use formatter to parse 2 digit year
// but that's a bit much for our use case
#define CENTURY (2000)
	return std::chrono::year_month_day{
			std::chrono::year(std::stoi(s.substr(4)) + CENTURY), 
			std::chrono::month(std::stoi(s.substr(2,  2))),
			std::chrono::day(std::stoi(s.substr(0,  2)))
	};
#undef CENTURY
}

bool nmea::valid(const std::string_view& sv) {
	assert(sv.starts_with('A') ||
			sv.starts_with('V'));
	return sv.starts_with('A');
}

quality nmea::qual(const std::string_view& sv) {
	// the typecast from int to enem<unsigned int> below is confirmed
	// to be safe in this case
	// https://www.modernescpp.com/index.php/strongly-typed-enums/
	return static_cast<quality>(std::stoi(std::string(sv)));
}

// $GPGLL,<Lat>,<N/S>,<Long>,<E/W>,<Timestamp>,<Status>,<mode indicator>*<checksum><cr><lf>

gll::gll_result gll::from_data(const std::string& data) {
	unsigned int field = 0;
	gll::gll_result gll;
    for (const auto word : std::views::split(data, delim)) {
    	switch (field) {
    	case 0: // talker id
    		gll.result.source = nmea::talker(std::string_view(word));
    		break;
    	case 1: // latitude
    		gll.result.lat = nmea::coord(2, std::string_view(word));
    		break;
    	case 2: // latitude direction
    		if (nmea::dir(std::string_view(word)) == direction::s) {
    			gll.result.lat = gll.result.lat * -1;
    		}
    		break;
    	case 3: // longitude
    		gll.result.lon = nmea::coord(3, std::string_view(word));
    		break;
    	case 4: // longitude direction
    		if (nmea::dir(std::string_view(word)) == direction::w) {
    			gll.result.lon = gll.result.lon * -1;
    		}
    		break;
    	case 5: // timestamp
    		gll.result.t = nmea::time(std::string_view(word));
    		break;
    	case 6: // valid
    		gll.result.valid = nmea::valid(std::string_view(word));
    		break;
    	default: // skip 7
    		break;
    	}
    	field++;
    }
	gll.result.success = (field == 8); // everything parsed
	return gll; 
}

// $GPGGA,<Timestamp>,<Lat>,<N/S>,<Long>,<E/W>,<GPSQual>,<Sats>,<HDOP>,<Alt>,<AltVal>,<GeoSep>,
// <GeoVal>,<DGPSAge>,<DGPSRef>*<checksum><cr><lf>
gga::gga_result gga::from_data(const std::string& data) {
	unsigned int field = 0;
	gga_result gga;
    for (const auto word : std::views::split(data, delim)) {
    	switch (field) {
    	case 0: // talker id
    		gga.result.source = nmea::talker(std::string_view(word));
    		break;
    	case 1: // timestamp
    		gga.result.t = nmea::time(std::string_view(word));
    		break;
    	case 2: // latitude
    		gga.result.lat = nmea::coord(2, std::string_view(word));
    		break;
    	case 3: // latitude direction
    		if (nmea::dir(std::string_view(word)) == direction::s) {
    			gga.result.lat = gga.result.lat * -1;
    		}
    		break;
    	case 4: // longitude
    		gga.result.lon = nmea::coord(3, std::string_view(word));
    		break;
    	case 5: // longitude direction
    		if (nmea::dir(std::string_view(word)) == direction::w) {
    			gga.result.lon = gga.result.lon * -1;
    		}
    		break;
    	case 6: // qual
    		gga.result.qual = nmea::qual(std::string_view(word));
    		break;
    	case 7: // sats
			gga.result.sats = std::stoi(std::string(std::string_view(word)));
    		break;
    	case 9: // altitude (in meters)
			// a float (and a double) may not precisely represent the value in the message
			gga.result.alt = std::stof(std::string(std::string_view(word)), nullptr);
    		break;		
    	case 11: // geoid separation (in meters)
			// a float (and a double) may not precisely represent the value in the message
			gga.result.geosep = std::stof(std::string(std::string_view(word)), nullptr);
    		break;		
    	default: // skip 8, 10, 12 .. 15
    		break;
    	}
    	field++;
    }
	gga.result.success = (field == 15); // everything parsed
	return gga; 
}

// $GNGSA,A,3,15,18,,,,,,,,,,,4.7,3.7,2.9*2D
// $GNGSA,A,3,73,65,81,,,,,,,,,,4.7,3.7,2.9*2E
gsa::gsa_result gsa::from_data(const std::string& data) {
	unsigned int field = 0;
	gsa_result gsa;
	std::string_view v = std::string_view(data).substr(0, data.find('*'));
    for (const auto word : std::views::split(v, delim)) {
    	switch (field) {
    	case 0: // talker id
    		gsa.result.source = nmea::talker(std::string_view(word));
    		break;
    	case 3:
    	case 4:
    	case 5:
    	case 6:
    	case 7:
    	case 8:
    	case 9:
    	case 10:
    	case 11:
    	case 12:
    	case 13:
    	case 14:
    		if (std::string_view(word).length() == 0) {
    			gsa.result.sats[field - 3] = 0;
    		} else {
    			gsa.result.sats[field - 3] =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	case 15:
    		gsa.result.system_id = nmea::system(std::string_view(word));
    		break;
    	default: // skip 1,2, 16, 17
    		break;
    	}
    	field++;
    }
	gsa.result.success = (field == 18); // everything parsed
	return gsa; 
}

// $GPGSV,3,1,11,13,79,310,,14,53,113,,05,51,214,,30,47,067,*72
// $GPGSV,3,2,11,15,45,295,24,22,44,145,,20,27,192,,07,16,064,*7A
// $GPGSV,3,3,11,18,16,298,25,24,08,249,,08,08,029,18,,,,*40
// $GLGSV,2,1,08,72,79,113,,74,77,084,,75,38,202,,65,37,317,28*68
// $GLGSV,2,2,08,73,34,040,35,71,28,130,,81,13,333,24,82,08,017,*68
gsv::gsv_result gsv::from_data(const std::string& data) {
	unsigned int field = 0;
	gsv_result gsv;
	std::string_view v = std::string_view(data).substr(0, data.find('*'));
    for (const auto word : std::views::split(v, delim)) {
    	switch (field) {
    	case 0: // talker id
    		gsv.result.source = nmea::talker(std::string_view(word));
    		break;
    	case 4:
    	case 8:
    	case 12:
    	case 16:
    		if (std::string_view(word).length() == 0) {
    			gsv.result.sats[(field - 4) / 4 ].prn = 0;
    		} else {
    			gsv.result.sats[(field - 4) / 4 ].prn =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	case 5:
    	case 9:
    	case 13:
    	case 17:
    		if (std::string_view(word).length() == 0) {
    			gsv.result.sats[(field - 5) / 4 ].elev = 0;
    		} else {
    			gsv.result.sats[(field - 5) / 4 ].elev =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	case 6:
    	case 10:
    	case 14:
    	case 18:
    		if (std::string_view(word).length() == 0) {
    			gsv.result.sats[(field - 6) / 4 ].azim = 0;
    		} else {
    			gsv.result.sats[(field - 6) / 4 ].azim =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	case 7:
    	case 11:
    	case 15:
    	case 19:
    		if (std::string_view(word).length() == 0) {
    			gsv.result.sats[(field - 7) / 4 ].snr = 0;
    		} else {
    			gsv.result.sats[(field - 7) / 4 ].snr =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	default: // skip 1, 2, 3
    		break;
    	}
    	field++;
    }
	gsv.result.success = (field == 20); // everything parsed
	return gsv; 
}

// $GPRMC,185427.150,V,5051.83778,N,00422.55809,E,,,240724,,,N*7F
rmc::rmc_result rmc::from_data(const std::string& data) {
	unsigned int field = 0;
	rmc_result rmc;
    for (const auto word : std::views::split(data, delim)) {
    	switch (field) {
    	case 0: // talker id
    		rmc.result.source = nmea::talker(std::string_view(word));
    		break;
    	case 1: // timestamp
    		rmc.result.t = nmea::time(std::string_view(word));
    		break;
    	case 2: // valid
    		rmc.result.valid = nmea::valid(std::string_view(word));
    		break;
    	case 3: // latitude
    		rmc.result.lat = nmea::coord(2, std::string_view(word));
    		break;
    	case 4: // latitude direction
    		if (nmea::dir(std::string_view(word)) == direction::s) {
    			rmc.result.lat = rmc.result.lat * -1;
    		}
    		break;
    	case 5: // longitude
    		rmc.result.lon = nmea::coord(3, std::string_view(word));
    		break;
    	case 6: // longitude direction
    		if (nmea::dir(std::string_view(word)) == direction::w) {
    			rmc.result.lon = rmc.result.lon * -1;
    		}
    		break;
    	case 9: // dqtestamp
    		rmc.result.d = nmea::date(std::string_view(word));
    		break;
    	default: // skip 7, 8, 10, 11, 12
    		break;
    	}
    	field++;
    }
	rmc.result.success = (field == 13); // everything parsed
	return rmc; 
}

} // namespace nmea
