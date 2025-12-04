/*
 * main.cpp
 *
 *  Created on, //29 jul. 2024
 *      Author, //jancu
 */


#include <string>
#include <vector>
#include <iostream>

import nmea;

void test_gll() {
	std::string reply = "$GPGLL,5051.83778,N,00422.55809,S,185427.150,V,N*4F";
    if (auto result = nmea::gll::from_data(reply)) {
    	std::cout <<
    		"GLL " << std::endl <<
    		"source: " << result.result.source << ". " <<
			"lat: " << result.result.lat << " lon: " << result.result.lon << ". " <<
    		result.result.t << ". " <<
			"valid: " << result.result.valid  << ". " <<
			std::endl;
	}

    return;
}

void test_gga() {
	std::string reply = "$GPGGA,191237.000,5051.78066,N,00422.57079,E,1,05,3.7,027.26,M,47.3,M,,*65";
    if (auto result = nmea::gga::from_data(reply)) {
	    std::cout <<
    		"GGA " << std::endl <<
    		"source: " << result.result.source << ". " <<
			"lat: " << result.result.lat << " lon: " << result.result.lon << ". " <<
    		result.result.t << ". " <<
			"qual: " << result.result.qual << ", " <<
			"sats: " << result.result.sats << ". " <<
			std::endl;
	}

    return;
}

void test_gsa() {
	std::vector<std::string> replies {
		"$GNGSA,A,3,15,18,,,,,,,,,,,4.7,3.7,2.9*2D",
		"$GNGSA,A,3,73,65,81,,,,,,,,,,4.7,3.7,2.9*2E"
	};

	for(auto r : replies) {
		if (auto result = nmea::gsa::from_data(r)) {
		    std::cout <<
	    		"GSA " << std::endl <<
	    		"source: " << result.result.source << ". " << std::endl <<
	       		"system: " << result.result.system_id << ". " << std::endl;
		    for(const auto s : result.result.sats) {
	    		std::cout << "sat prn: " << s << "." <<
				std::endl;
	    	}
		}
	}
    return;
}


void test_gsv() {
	std::vector<std::string> replies {
		"$GPGSV,3,1,11,13,79,310,,14,53,113,,05,51,214,,30,47,067,*72",
		"$GPGSV,3,2,11,15,45,295,24,22,44,145,,20,27,192,,07,16,064,*7A",
		"$GPGSV,3,3,11,18,16,298,25,24,08,249,,08,08,029,18,,,,*40",
		"$GLGSV,2,1,08,72,79,113,,74,77,084,,75,38,202,,65,37,317,28*68",
		"$GLGSV,2,2,08,73,34,040,35,71,28,130,,81,13,333,24,82,08,017,*68"
	};

	for(auto r : replies) {
		if (auto result = nmea::gsv::from_data(r)) {
		    std::cout <<
	    		"GSV " << std::endl <<
	    		"source: " << result.result.source << ". " << std::endl;
		    for(const auto s : result.result.sats) {
	    		std::cout << "sat prn: " << s.prn << ", elev: " <<
	    			s.elev << ", azim: " << s.azim << ", snr: " << s.snr << "." <<
					std::endl;
	    	}
		}
	}
    return;
}

void test_rmc() {
	std::string reply = "$GPRMC,185427.150,V,5051.83778,N,00422.55809,E,,,240724,,,N*7F";
    if (auto result = nmea::rmc::from_data(reply)) {
	    std::cout <<
    		"RMC " << std::endl <<
    		"source: " << result.result.source << ". " <<
			"lat: " << result.result.lat << " lon: " << result.result.lon << ". " <<
    		result.result.t << ". " <<
    		result.result.d << ". " <<
			"valid: " << result.result.valid  << ". " <<
			std::endl;
	}
    return;
}

int main() {
	test_gll();
	test_gga();
	test_gsa();
	test_gsv();
	test_rmc();

	return 0;
}
