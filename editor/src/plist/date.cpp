#include "date.hpp"

#include <stdexcept>

namespace ek::plist {

date_t::date_t() {
    set_to_current_time();
}

date_t::date_t(int month, int day, int year, int hour24, int minute, int second, bool utc) {
    set(month, day, year, hour24, minute, second, utc);
}

void date_t::set(int month, int day, int year, int hour24, int minute, int second, bool utc) {
    struct tm tm_time{};
    tm_time.tm_hour = hour24;
    tm_time.tm_mday = day;
    tm_time.tm_year = year - 1900;
    tm_time.tm_sec = second;
    tm_time.tm_mon = month - 1;
    tm_time.tm_min = minute;

    //get proper day light savings.

    time_t loc = time(nullptr);
    struct tm tm_loc = *localtime(&loc);
    //std::cout<<"tmLoc.tm_isdst = "<<tmLoc.tm_isdst<<std::endl;
    tm_time.tm_isdst = tm_loc.tm_isdst;

    if (utc) {
        //time_ = timegm(&tm_time);

        tm_time.tm_isdst = 0;
        time_ = mktime(&tm_time);
        if (time_ < -1) {
            throw std::runtime_error("plist::date_t::set() date invalid");
        }

        // don't have timegm for all systems so here's a portable way to do it.

        struct tm tm_time_temp{};
#if defined(_WIN32) || defined(_WIN64)
        gmtime_s(&tm_time_temp, &time_);
#else
        gmtime_r(&time_, &tm_time_temp);
#endif

        time_t time_temp = mktime(&tm_time_temp);

        time_t diff = time_ - time_temp;
        time_ += diff;
    } else {
        time_ = mktime(&tm_time);
        if (time_ < -1) {
            throw std::runtime_error("plist::date_t::set() date invalid");
        }
    }
}

void date_t::set_to_current_time() {
    time_ = time(nullptr);
}

time_t date_t::seconds_since_date(const date_t& start_date) const {
    return time_ - start_date.time_as_epoch();
}

// returns -1 : first < second
//          0 : first = second
//          1 : first > second
int date_t::compare(const date_t& first, const date_t& second) {
    if (first.time_as_epoch() < second.time_as_epoch())
        return -1;
    else if (first.time_as_epoch() == second.time_as_epoch())
        return 0;
    else
        return 1;
}

bool date_t::operator>(const date_t& rhs) const {
    return compare(*this, rhs) == 1;
}

bool date_t::operator<(const date_t& rhs) const {
    return compare(*this, rhs) == -1;
}

bool date_t::operator==(const date_t& rhs) const {
    return compare(*this, rhs) == 0;
}

// iso 8601 date string convention
std::string date_t::time_as_xml_convention() const {
    char result[21];
    struct tm tm_time{};

    // use thread safe versions here.  Notice that arguments
    // are reversed for windows version
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_time, &time_);
#else
    gmtime_r(&time_, &tm_time);
#endif
    // %F and %T not portable so using %Y-%m-%d and %H:%M:%S instead
    strftime(result, 21, "%Y-%m-%dT%H:%M:%SZ", &tm_time);
    return std::string(result);
}

// iso 8601 date string convention
void date_t::set_time_from_xml_convention(const std::string& time_string) {
    int month, day, year, hour24, minute, second;

    // parse date string.  E.g.  2011-09-25T02:31:04Z
    sscanf(time_string.c_str(), "%4d-%2d-%2dT%2d:%2d:%2dZ", &year, &month, &day, &hour24, &minute, &second);
    set(month, day, year, hour24, minute, second, true);
}

// Apple epoch is # of seconds since  01-01-2001. So we need to add the
// number of seconds since 01-01-1970 which is proper unix epoch

void date_t::set_time_from_apple_epoch(double apple_time) {
    time_ = time_t(978307200 + apple_time);
}

time_t date_t::time_as_epoch() const {
    return time_;
}

// We need to subtract the number of seconds between 01-01-2001 and
// 01-01-1970 to get Apple epoch from unix epoch
double date_t::time_as_apple_epoch() const {
    return ((double)time_ - 978307200);
}

}