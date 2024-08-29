/*
 * nmea.cpp
 *
 *  Created on: 29 jul. 2024
 *      Author: jancu
 */

#include <cassert>
#include <ranges>
#include <cmath>
#include "nmea.h"

using std::operator""sv;
constexpr auto delim{","sv};

namespace nmea {

nmea::talker_id nmea::talker(const std::string_view& sv) {
	talker_id id = gps;
	if (sv.starts_with("$GP")) {
		id = gps;
	} else if (sv.starts_with("$GL")) {
		id = glonass;
	} else if (sv.starts_with("$GA")) {
		id = galileo;
	} else if (sv.starts_with("$BD")) {
		id = beidou;
	} else if (sv.starts_with("$QZ")) {
		id = qzss;
	} else if (sv.starts_with("$GN")) {
		id = multiconstellation;
	}
	return id;
}

nmea::talker_id nmea::system(const std::string_view& sv) {
	talker_id id = gps;
	if (sv.starts_with("1")) {
		id = gps;
	} else if (sv.starts_with("2")) {
		id = glonass;
	} else if (sv.starts_with("3")) {
		id = galileo;
	} else if (sv.starts_with("4")) {
		id = beidou;
	} else if (sv.starts_with("5")) {
		id = qzss;
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

nmea::direction nmea::dir(const std::string_view& sv) {
	assert(sv.starts_with('N') ||
			sv.starts_with('S') ||
			sv.starts_with('E') ||
			sv.starts_with('W'));
	direction d = n;
	if (sv.starts_with('N')) {
		d = n;
	} else if (sv.starts_with('S')) {
		d = s;
	} else if (sv.starts_with('E')) {
		d = e;
	} else if (sv.starts_with('W')) {
		d = w;
	}
	return d;
}

void nmea::time(const std::string_view& sv, time_t& t) {
	std::string s(sv);
		t = time_t{
			std::chrono::hours(std::stoi(s.substr(0,  2))) +
			std::chrono::minutes(std::stoi(s.substr(2,  2))) +
		    std::chrono::seconds(std::stoi(s.substr(4,  2))) +
			std::chrono::milliseconds(std::stoi(s.substr(7)))
	};
}

void nmea::date(const std::string_view& sv, std::chrono::year_month_day& d) {
	std::string s(sv);
	d = std::chrono::year_month_day{
			std::chrono::year(std::stoi(s.substr(4)) + 2000), // TODO Y2.1K warning
			std::chrono::month(std::stoi(s.substr(2,  2))),
			std::chrono::day(std::stoi(s.substr(0,  2)))
	};
}

bool nmea::valid(const std::string_view& sv) {
	assert(sv.starts_with('A') ||
			sv.starts_with('V'));
	return sv.starts_with('A');
}

nmea::quality nmea::qual(const std::string_view& sv) {
	// the typecast from int to enem<unsigned int> below is confirmed
	// to be safe in this case
	// https://www.modernescpp.com/index.php/strongly-typed-enums/
	return static_cast<nmea::quality>(std::stoi(std::string(sv)));
}

// $GPGLL,<Lat>,<N/S>,<Long>,<E/W>,<Timestamp>,<Status>,<mode indicator>*<checksum><cr><lf>
bool gll::from_data(const std::string& data, gll& gll) {
	unsigned int field = 0;
    for (const auto word : std::views::split(data, delim)) {
    	switch (field) {
    	case 0: // talker id
    		gll.source = nmea::talker(std::string_view(word));
    		break;
    	case 1: // latitude
    		gll.lat = nmea::coord(2, std::string_view(word));
    		break;
    	case 2: // latitude direction
    		if (nmea::dir(std::string_view(word)) == nmea::s) {
    			gll.lat = gll.lat * -1;
    		}
    		break;
    	case 3: // longitude
    		gll.lon = nmea::coord(3, std::string_view(word));
    		break;
    	case 4: // longitude direction
    		if (nmea::dir(std::string_view(word)) == nmea::w) {
    			gll.lon = gll.lon * -1;
    		}
    		break;
    	case 5: // timestamp
    		nmea::time(std::string_view(word), gll.t);
    		break;
    	case 6: // valid
    		gll.valid = nmea::valid(std::string_view(word));
    		break;
    	default: // skip 7
    		break;
    	}
    	field++;
    }
	return (field == 8); // everything parsed
}

// $GPGGA,<Timestamp>,<Lat>,<N/S>,<Long>,<E/W>,<GPSQual>,<Sats>,<HDOP>,<Alt>,<AltVal>,<GeoSep>,
// <GeoVal>,<DGPSAge>,<DGPSRef>*<checksum><cr><lf>
bool gga::from_data(const std::string& data, gga& gga) {
	unsigned int field = 0;
    for (const auto word : std::views::split(data, delim)) {
    	switch (field) {
    	case 0: // talker id
    		gga.source = nmea::talker(std::string_view(word));
    		break;
    	case 1: // timestamp
    		nmea::time(std::string_view(word), gga.t);
    		break;
    	case 2: // latitude
    		gga.lat = nmea::coord(2, std::string_view(word));
    		break;
    	case 3: // latitude direction
    		if (nmea::dir(std::string_view(word)) == nmea::s) {
    			gga.lat = gga.lat * -1;
    		}
    		break;
    	case 4: // longitude
    		gga.lon = nmea::coord(3, std::string_view(word));
    		break;
    	case 5: // longitude direction
    		if (nmea::dir(std::string_view(word)) == nmea::w) {
    			gga.lon = gga.lon * -1;
    		}
    		break;
    	case 6: // qual
    		gga.qual = nmea::qual(std::string_view(word));
    		break;
    	case 7: // sats
			gga.sats = std::stoi(std::string(std::string_view(word)));
    		break;
    	case 9: // altitude
			// a float (and a double) may not precisely represent the value in the message
			gga.alt = std::stof(std::string(std::string_view(word)), nullptr);
    		break;		
    	default: // skip 8, 10 .. 15
    		break;
    	}
    	field++;
    }
	return (field == 15); // everything parsed
}

// $GNGSA,A,3,15,18,,,,,,,,,,,4.7,3.7,2.9*2D
// $GNGSA,A,3,73,65,81,,,,,,,,,,4.7,3.7,2.9*2E
bool gsa::from_data(const std::string& data, gsa& gsa) {
	unsigned int field = 0;
	std::string_view v = std::string_view(data).substr(0, data.find('*'));
    for (const auto word : std::views::split(v, delim)) {
    	switch (field) {
    	case 0: // talker id
    		gsa.source = nmea::talker(std::string_view(word));
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
    			gsa.sats[field - 3] = 0;
    		} else {
    			gsa.sats[field - 3] =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	case 15:
    		gsa.system_id = nmea::system(std::string_view(word));
    		break;
    	default: // skip 1,2, 16, 17
    		break;
    	}
    	field++;
    }
	return (field == 18); // everything parsed
}

// $GPGSV,3,1,11,13,79,310,,14,53,113,,05,51,214,,30,47,067,*72
// $GPGSV,3,2,11,15,45,295,24,22,44,145,,20,27,192,,07,16,064,*7A
// $GPGSV,3,3,11,18,16,298,25,24,08,249,,08,08,029,18,,,,*40
// $GLGSV,2,1,08,72,79,113,,74,77,084,,75,38,202,,65,37,317,28*68
// $GLGSV,2,2,08,73,34,040,35,71,28,130,,81,13,333,24,82,08,017,*68
bool gsv::from_data(const std::string& data, gsv& gsv) {
	unsigned int field = 0;
	std::string_view v = std::string_view(data).substr(0, data.find('*'));
    for (const auto word : std::views::split(v, delim)) {
    	switch (field) {
    	case 0: // talker id
    		gsv.source = nmea::talker(std::string_view(word));
    		break;
    	case 4:
    	case 8:
    	case 12:
    	case 16:
    		if (std::string_view(word).length() == 0) {
    			gsv.sats[(field - 4) / 4 ].prn = 0;
    		} else {
    			gsv.sats[(field - 4) / 4 ].prn =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	case 5:
    	case 9:
    	case 13:
    	case 17:
    		if (std::string_view(word).length() == 0) {
    			gsv.sats[(field - 5) / 4 ].elev = 0;
    		} else {
    			gsv.sats[(field - 5) / 4 ].elev =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	case 6:
    	case 10:
    	case 14:
    	case 18:
    		if (std::string_view(word).length() == 0) {
    			gsv.sats[(field - 6) / 4 ].azim = 0;
    		} else {
    			gsv.sats[(field - 6) / 4 ].azim =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	case 7:
    	case 11:
    	case 15:
    	case 19:
    		if (std::string_view(word).length() == 0) {
    			gsv.sats[(field - 7) / 4 ].snr = 0;
    		} else {
    			gsv.sats[(field - 7) / 4 ].snr =
    					std::stoi(std::string(std::string_view(word)));
    		}
    		break;
    	default: // skip 1, 2, 3
    		break;
    	}
    	field++;
    }
	return (field == 20); // everything parsed
}

// $GPRMC,185427.150,V,5051.83778,N,00422.55809,E,,,240724,,,N*7F
bool rmc::from_data(const std::string& data, rmc& rmc) {
	unsigned int field = 0;
    for (const auto word : std::views::split(data, delim)) {
    	switch (field) {
    	case 0: // talker id
    		rmc.source = nmea::talker(std::string_view(word));
    		break;
    	case 1: // timestamp
    		nmea::time(std::string_view(word), rmc.t);
    		break;
    	case 2: // valid
    		rmc.valid = nmea::valid(std::string_view(word));
    		break;
    	case 3: // latitude
    		rmc.lat = nmea::coord(2, std::string_view(word));
    		break;
    	case 4: // latitude direction
    		if (nmea::dir(std::string_view(word)) == nmea::s) {
    			rmc.lat = rmc.lat * -1;
    		}
    		break;
    	case 5: // longitude
    		rmc.lon = nmea::coord(3, std::string_view(word));
    		break;
    	case 6: // longitude direction
    		if (nmea::dir(std::string_view(word)) == nmea::w) {
    			rmc.lon = rmc.lon * -1;
    		}
    		break;
    	case 9: // dqtestamp
    		nmea::date(std::string_view(word), rmc.d);
    		break;
    	default: // skip 7, 8, 10, 11, 12
    		break;
    	}
    	field++;
    }
	return (field == 13); // everything parsed
}

} // namespace nmea
