#pragma once

#include <cstdio>
#include <ctime>

namespace Web::Utils {

    /**
     * @brief Parses a ISO_8601 string into a time_t struct (UTC).
     *
     * @param dateStr
     * @return std::time_t seconds from 1/1/1970
     */
    std::time_t inline datetimeISO_8601ToTime(const char* dateStr) {
        // e.g. "2011-10-05T14:48:00.000Z"
        struct tm t;
        int success =
            sscanf(dateStr, "%d-%d-%dT%d:%dZ", /* */
                   &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min);
        if (success != 5) {
            return 0;
        }

        /* compensate expected ranges */
        t.tm_year = t.tm_year - 1900;
        t.tm_mon = t.tm_mon - 1;
        t.tm_sec = 0;
        t.tm_wday = 0;
        t.tm_yday = 0;
        t.tm_isdst = 0;

        time_t localTime = mktime(&t);
        time_t utcTime = localTime - timezone;

        return utcTime;
    }
}  // namespace Web::Utils