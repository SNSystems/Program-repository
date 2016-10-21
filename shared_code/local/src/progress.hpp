// Copyright (c) 2016 by Sony Computer Entertainment Inc.
// This file is subject to the terms and conditions defined in file
// 'LICENSE.txt', which is part of this source code package.

#ifndef SCANLIB_PROGRESS_HPP
#define SCANLIB_PROGRESS_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
//#include <vector>
#include <boost/circular_buffer.hpp>

namespace details {
    // ****************
    // * progress_bar *
    // ****************
    class progress_bar {
    public:
        explicit progress_bar (unsigned width = 40);
        void set_value (float value);
        std::ostream & write (std::ostream & os) const;

    private:
        unsigned const width_;
        float value_ = 0.0;
    };

    std::ostream & operator << (std::ostream & os, progress_bar const & prog);


    // *************************
    // * hours_minutes_seconds *
    // *************************
    class hours_minutes_seconds {
    public:
        hours_minutes_seconds (float secs);

        std::string to_string () const;
        std::ostream & write (std::ostream & os) const;

        unsigned hours () const { return hours_; }
        unsigned minutes () const { return minutes_; }
        unsigned seconds () const { return seconds_; }

    private:
        unsigned hours_;
        unsigned minutes_;
        unsigned seconds_;

        static constexpr auto const minute_threshold_for_no_seconds = 2U;
        static constexpr auto const seconds_in_minute = 60U;
        static constexpr auto const minutes_in_hour = 60U;
        static constexpr auto const seconds_in_hour = 60U * 60U;
    };

    std::ostream & operator << (std::ostream & os, hours_minutes_seconds const & hms);


    // *****************************
    // * time_remaining_calculator *
    // *****************************
    class time_remaining_calculator {
    public:
        explicit time_remaining_calculator (std::size_t size);

        float update (float speed);

        float mean () const { return stats_.mean (); }
        std::size_t samples () const { return stats_.samples (); }
        float standard_deviation () const { return stats_.stddev (); }

        class stats {
        public:
            // Adds 'x' to the sample collection.
            void push (float x);
            // Removes 'x' from the sample collection.
            void pop (float x);

            std::size_t samples () const { return samples_; }
            float mean () const { return mean_; }
            float stddev () const;

        private:
            float variance () const;

            std::size_t samples_ = 0U;
            float mean_ = 0.0F;
            float s_ = 0.0F;
        };

    private:
        boost::circular_buffer<float> buffer_;
        stats stats_;
    };
}

// *********
// * timer *
// *********
class timer {
public:
    timer (std::function <void ()> proc, std::chrono::milliseconds interval);
    ~timer () noexcept;
    void clear ();
    
private:
    void run (std::function <void ()> proc, std::chrono::milliseconds interval) noexcept;

    std::atomic <bool> running_ {true};
    std::thread thread_;
};

typedef std::shared_ptr <timer>  timer_ptr;

timer_ptr setInterval (std::function <void ()> proc, std::chrono::milliseconds interval);
void clearInterval (timer_ptr timer) noexcept;


// ****************
// * updater_intf *
// ****************
class updater_intf {
public:
    virtual ~updater_intf () noexcept = 0;

    virtual updater_intf & completed (unsigned /*c*/) { return *this; }
    virtual updater_intf & completed_incr (unsigned /*by*/ = 1) { return *this; }

    virtual updater_intf & total (unsigned /*value*/) { return *this; }
    virtual updater_intf & total_incr (unsigned /*by*/ = 1) { return *this; }

    virtual updater_intf & run () { return *this; }
};


// ******************
// * silent_updater *
// ******************
class silent_updater final : public updater_intf {
public:
    explicit silent_updater (char const * /*message*/ = nullptr);
    ~silent_updater () noexcept;
};


// ***********
// * updater *
// ***********
class updater final : public updater_intf {
public:
    explicit updater (char const * message = nullptr, unsigned total = 0);
    ~updater () noexcept override;

    unsigned completed () const;
    updater & completed (unsigned c) override;
    updater & completed_incr (unsigned by = 1) override;

    unsigned total () const;
    updater & total (unsigned value) override;
    updater & total_incr (unsigned by = 1) override;

    updater & run () override;

private:
    std::ostream & time_left (float const average, std::ostream & os) const;
    bool have_reliable_estimate (float const average) const;

    void update ();

    mutable std::mutex mutex_;

    details::progress_bar bar_;
    // time remaining estimates will be based on the completion rate that we've achieved 
    // over the last 30 seconds.
    details::time_remaining_calculator time_remaining_;
    unsigned completed_ = 0;
    unsigned total_ = 0;

    unsigned old_completed_ = 0;
    std::size_t prev_width_ = 0;

    timer_ptr timer_;
};
#endif // SCANLIB_PROGRESS_HPP
// eof progress.hpp
