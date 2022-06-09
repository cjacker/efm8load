#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"
#else
#include "Types.h"
#endif

#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <time.h>

/////////////////////////////////////////////////////////////////////////////
// Custom User Options
/////////////////////////////////////////////////////////////////////////////

// Enable or disable the profiler: TRUE or FALSE
#define ENABLE_PROFILER			TRUE

// CProfiler object symbol name used in DECLARE_PROFILER()
// Add this to your watch window to view timing information
// while in debug mode
#define PROFILER_NAME			profiler

// double total elapsed time used in DECLARE_TOTAL()
// Add this to your watch window to view total elapsed time
// while in debug mode
#define TOTAL_NAME			total

// Units of time: TIME_SECONDS, TIME_MILLISECONDS, or TIME_MICROSECONDS
#define TIME_UNITS			TIME_SECONDS

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

#define TIME_SECONDS			1
#define TIME_MILLISECONDS		1000
#define TIME_MICROSECONDS		1000000

/////////////////////////////////////////////////////////////////////////////
// Macros
/////////////////////////////////////////////////////////////////////////////

// START()		- Reset the CProfiler object and record starting time
// LAP(expression)	- Execute the specified expression
//			- Record time elapsed from previous LAP() or START() call
// STOP(expression)	- Execute the specified expression
//			- Record time elapsed from previous LAP() or START() call
//			  and return the total elapsed time
// DUMP(filename)	- Dump profiler information to the specified file

#if (ENABLE_PROFILER)
#define START()				PROFILER_NAME.Start("Start");
#define LAP(expression)			expression; PROFILER_NAME.Lap(#expression);
#define STOP(expression)		expression; TOTAL_NAME = PROFILER_NAME.Stop(#expression);
#define DUMP(filename)			PROFILER_NAME.Dump(filename);

#define DECLARE_PROFILER()		CProfiler PROFILER_NAME;
#define DECLARE_TOTAL()			double TOTAL_NAME = 0.0f;
#else
#define START()
#define LAP(expression)			expression;
#define STOP(expression)		expression;
#define DUMP(filename)

#define DECLARE_PROFILER()
#define DECLARE_TOTAL()
#endif

/////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////

struct ProfilerTimestamp
{
    double elapsed;
    std::string comment;
    double time;

    ProfilerTimestamp(double t, double e, std::string c = std::string())
    : time(t)
    {
        elapsed = e;
        comment = c;
    }
};

/////////////////////////////////////////////////////////////////////////////
// CProfiler Class
/////////////////////////////////////////////////////////////////////////////

class CProfiler
{
    // Constructor
public:

    CProfiler()
    {
        // Get the resolution of the system clock
        clock_getres(CLOCK_MONOTONIC, &m_resolution);
        
        // Set initial capacity to 1000 to reduce time variations
        // due to memory copy
        m_timestamps.reserve(1000);
    }

    // Public Methods
public:

    void Start(std::string comment = "")
    {
        m_timestamps.clear();

        m_timestamps.push_back(ProfilerTimestamp(GetTime(), 0, comment));
    }

    // Return total time from Start to Stop

    double Stop(std::string comment = "")
    {
        double total;

        Lap(comment);
        total = m_timestamps.back().time - m_timestamps.front().time;

        return total;
    }

    // Return total time from Start to Stop

    double Elapsed()
    {
        double total;

        total = m_timestamps.back().time - m_timestamps.front().time;

        return total;
    }

    // Return elapsed time from last Lap or Start

    double Lap(std::string comment = "")
    {
        double time = GetTime();
        double elapsed = time - m_timestamps.back().time;

        m_timestamps.push_back(ProfilerTimestamp(time, elapsed, comment));

        return elapsed;
    }

    // System uptime

    double GetTime()
    {
        double time;
        timespec tp;
        
        clock_gettime(CLOCK_MONOTONIC, &tp);
        
        time = ((double)TIME_UNITS * (tp.tv_sec + (tp.tv_nsec / 1000000000.0f)));

        return time;
    }

    void Dump(std::string filename)
    {
        std::fstream file;

        // Determine the unit string
        std::string timeUnits;

#if (TIME_UNITS == TIME_SECONDS)
        timeUnits = "s";
#elif (TIME_UNITS == TIME_MILLISECONDS)
        timeUnits = "ms";
#elif (TIME_UNITS == TIME_MICROSECONDS)
        timeUnits = "us";
#endif

        // Open the file for output
        file.open(filename.c_str(), std::fstream::out);

        double resolution = (double)m_resolution.tv_sec + (m_resolution.tv_nsec / 1000000000.0f);
        
        // Log profiler header text
        file << "Profiler Dump" << std::endl;
        file << "-------------" << std::endl << std::endl;
        file << "High Performance Counter Precision: " << ((double) TIME_UNITS * resolution) << " " << timeUnits << std::endl;
        file << std::endl;

        size_t longestString = 0;

        // Get the length of the longest comment string
        for (size_t i = 0; i < m_timestamps.size(); i++)
        {
            longestString = MAX(longestString, m_timestamps[i].comment.size());
        }

        // Set the number of digits to display after the decimal point
        // to display up to nanoseconds
        file.setf(std::ios::fixed);
#if (TIME_UNITS == TIME_SECONDS)
        file.precision(9);
#elif (TIME_UNITS == TIME_MILLISECONDS)
        file.precision(6);
#elif (TIME_UNITS == TIME_MICROSECONDS)
        file.precision(3);
#endif

        // Output the comment and elapsed time for each timestamp entry
        for (size_t i = 0; i < m_timestamps.size(); i++)
        {
            file << "Timestamp " << std::setw(5) << i << ": ";

            file << "Elapsed: " << std::setw(13) << m_timestamps[i].elapsed << " " << timeUnits << " ";

            // Display the line comment and space all elapsed times exactly 3 characters after the longest string
            file << "Line: \"" << m_timestamps[i].comment << "\"";

            file << std::endl;
        }

        // Display the total time elapsed
        if (m_timestamps.size() > 0)
        {
            file << std::endl;
            file << "Total Time: " << (m_timestamps.back().time - m_timestamps.front().time) << " " << timeUnits << std::endl;
        }

        file.close();
    }

public:
    timespec m_resolution;
    std::vector<ProfilerTimestamp> m_timestamps;
};
