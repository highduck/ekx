#pragma once

#include <ctime>
#include <string>

namespace ek::plist {

/***
 * Date structure for PList
 */
class date_t {
public:
    date_t();

    date_t(int month, int day, int year, int hour24, int minute, int second, bool utc);

    void set(int month, int day, int year, int hour24, int minute, int second, bool utc);

    void set_to_current_time();

    [[nodiscard]] time_t seconds_since_date(const date_t& start_date) const;

    /***
     * Compare date objects
     *
     * @param first
     * @param second
     * @return  -1 : first < second
     *           0 : first = second
     *           1 : first > second
     */
    static int compare(const date_t& first, const date_t& second);

    bool operator>(const date_t& rhs) const;

    bool operator<(const date_t& rhs) const;

    bool operator==(const date_t& rhs) const;

    /***
     * @return iso 8601 date string convention
     */
    [[nodiscard]] std::string time_as_xml_convention() const;

    /***
     * @param time_string is iso 8601 date string convention
     */
    void set_time_from_xml_convention(const std::string& time_string);

    /***
     * Apple epoch is # of seconds since  01-01-2001. So we need to add the
     * number of seconds since 01-01-1970 which is proper unix epoch
     *
     * @param apple_time
     */
    void set_time_from_apple_epoch(double apple_time);

    /***
     * Apple epoch is # of seconds since  01-01-2001. So we need to add the
     * number of seconds since 01-01-1970 which is proper unix epoch
     *
     * @return
     */
    [[nodiscard]] time_t time_as_epoch() const;

    /***
     * We need to subtract the number of seconds between 01-01-2001 and
     * 01-01-1970 to get Apple epoch from unix epoch
     * @return
     */
    [[nodiscard]] double time_as_apple_epoch() const;

private:

    /***
     * timestamp stored as unix epoch, number of seconds since 01-01-1970
     */
    time_t time_{};
};

}


