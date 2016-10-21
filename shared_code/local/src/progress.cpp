#include "progress.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>

#include "print.hpp"

namespace details {
    //****************
    //* progress bar *
    //****************
    // (ctor)
    // ~~~~~~
    progress_bar::progress_bar (unsigned width)
            : width_ (width) {}

    // set value
    // ~~~~~~~~~
    void progress_bar::set_value (float value) {
        assert (value_ >= 0.0F && value_ <= 1.0F);
        value_ = std::max (0.0F, std::min (1.0F, value));
    }

    // write
    // ~~~~~
    std::ostream & progress_bar::write (std::ostream & os) const {
        assert (value_ >= 0.0F && value_ <= 1.0F);
        auto const num_hashes = static_cast<unsigned> (std::lround (value_ * width_));
        assert (num_hashes <= width_);

        os << '[';
        {
            std::ostream_iterator<char> out (os);
            std::fill_n (out, num_hashes, '#');
            std::fill_n (out, width_ - num_hashes, '-');
        }
        os << "] " << std::setw (3) << std::trunc (value_ * 100.0) << '%';
        return os;
    }

    // operator<<
    // ~~~~~~~~~~
    std::ostream & operator<< (std::ostream & os, progress_bar const & prog) {
        return prog.write (os);
    }
}

namespace details {

    // *********
    // * time remaining calculator::stats *
    // *********
    // push
    // ~~~~
    void time_remaining_calculator::stats::push (float x) {
        ++samples_;
        float const new_mean = mean_ + (x - mean_) / samples_;
        s_ += (x - mean_) * (x - new_mean);
        mean_ = new_mean;
    }
    // pop
    // ~~~
    void time_remaining_calculator::stats::pop (float x) {
        assert (samples_ > 0);
        if (samples_ <= 1U) {
            s_ = 0.0F;
            mean_ = 0.0F;
            samples_ = 0U;
        } else {
            auto const old = (samples_ * mean_ - x) / (samples_ - 1U);
            s_ -= (x - mean_) * (x - old);
            mean_ = old;
            --samples_;
        }
    }
    // variance
    // ~~~~~~~~
    float time_remaining_calculator::stats::variance () const {
        return samples_ > 1U ? s_ / samples_ : 0.0F;
    }
    // stddev
    // ~~~~~~
    float time_remaining_calculator::stats::stddev () const {
        return std::sqrt (this->variance ());
    }

    // *****************************
    // * time remaining calculator *
    // *****************************
    // (ctor)
    // ~~~~~~
    time_remaining_calculator::time_remaining_calculator (std::size_t size)
            : buffer_ (size) {
        assert (size > 0);
    }

    // update
    // ~~~~~~
    float time_remaining_calculator::update (float speed) {
        assert (speed >= 0);
        if (buffer_.full ()) {
            assert (buffer_.size () > 0);
            stats_.pop (buffer_.front ());
        }
        buffer_.push_back (speed);
        stats_.push (speed);
        return stats_.mean ();
    }
}

namespace details {
    // *********************
    // hours minutes seconds
    // *********************
    // (ctor)
    // ~~~~~~
    hours_minutes_seconds::hours_minutes_seconds (float secs) {
        assert (secs >= 0.0);

        seconds_ = static_cast<unsigned> (std::round (secs));

        hours_ = static_cast<unsigned> (std::trunc (seconds_ / seconds_in_hour));
        seconds_ -= hours_ * seconds_in_hour;
        minutes_ = static_cast<unsigned> (std::trunc (seconds_ / seconds_in_minute));
        seconds_ -= minutes_ * seconds_in_minute;
        assert (minutes_ < minutes_in_hour);
        assert (seconds_ < seconds_in_minute);

        if (minutes_ >= minute_threshold_for_no_seconds || hours_ > 0) {
            if (seconds_ >= 30) {
                minutes_ += 1;
                assert (minutes_ <= 60);
                if (minutes_ == 60) {
                    hours_ += 1;
                    minutes_ = 0;
                }
            }
            seconds_ = 0;
        }
    }

    // write
    // ~~~~~
    std::ostream & hours_minutes_seconds::write (std::ostream & os) const {
        bool has_content = false;
        if (hours_ == 0 && minutes_ == 0 && seconds_ == 0) {
            os << "0 seconds";
            has_content = true;
        } else {
            static char const * separator = ", ";
            if (hours_ > 0) {
                os << hours_ << ' ' << (hours_ != 1 ? "hours" : "hour");
                has_content = true;
            }
            if (minutes_ > 0) {
                if (has_content) {
                    os << separator;
                }
                os << minutes_ << ' ' << (minutes_ != 1 ? "minutes" : "minute");
                has_content = true;
            }
            if (seconds_ > 0) {
                if (has_content) {
                    os << separator;
                }
                os << seconds_ << ' ' << (seconds_ != 1 ? "seconds" : "second");
                has_content = true;
            }
        }

        assert (has_content);
        return os;
    }

    // to string
    // ~~~~~~~~~
    std::string hours_minutes_seconds::to_string () const {
        std::ostringstream str;
        this->write (str);
        return str.str ();
    }

    // operator<<
    // ~~~~~~~~~~
    std::ostream & operator<< (std::ostream & os, hours_minutes_seconds const & hms) {
        return hms.write (os);
    }
}

// *********
// * timer *
// *********
// (ctor)
// ~~~~~~
timer::timer (std::function<void()> proc, std::chrono::milliseconds interval)
        : thread_ (std::bind (&timer::run, this, proc, interval)) {}

// (dtor)
// ~~~~~~
timer::~timer () noexcept {
    try {
        this->clear ();
    } catch (...) {
    }
}

// clear
// ~~~~~
void timer::clear () {
    // Sets the running state to false: this will cause the worker thread to exit
    // the next time round its loop.
    running_ = false;
    thread_.join ();
}

// run
// ~~~
void timer::run (std::function<void()> proc, std::chrono::milliseconds interval) noexcept {
    try {
        while (running_) {
            std::this_thread::sleep_for (interval);
            if (running_) {
                proc ();
            }
        }
    } catch (...) {
        running_ = false;
    }
}

timer_ptr setInterval (std::function<void()> proc, std::chrono::milliseconds interval) {
    return std::make_shared<timer> (proc, interval);
}

void clearInterval (timer_ptr timer) noexcept {
    if (timer) {
        timer->clear ();
    }
}

// ****************
// * updater_intf *
// ****************
// (dtor)
// ~~~~~~
updater_intf::~updater_intf () noexcept {}

// ******************
// * silent_updater *
// ******************
// (ctor)
// ~~~~~~
silent_updater::silent_updater (char const *)
        : updater_intf () {}
// (dtor)
// ~~~~~~
silent_updater::~silent_updater () noexcept {}

// ***********
// * updater *
// ***********
// (ctor)
// ~~~~~~
updater::updater (char const * message, unsigned total)
        : updater_intf ()
        , time_remaining_ (30) // time estimates based on the last 30 seconds.
        , total_ (total) {
    if (message != nullptr) {
        print_cout_flush (message, '\n');
    }
}

// (dtor)
// ~~~~~~
updater::~updater () noexcept {
    try {
        clearInterval (timer_);
        timer_.reset ();
        this->update ();
        print_cout ("");
    } catch (...) {
    }
}

// completed
// ~~~~~~~~~
unsigned updater::completed () const {
    std::lock_guard<std::mutex> guard (mutex_);
    return completed_;
}
updater & updater::completed (unsigned c) {
    std::lock_guard<std::mutex> guard (mutex_);
    completed_ = c;
    assert (completed_ <= total_);
    return *this;
}
updater & updater::completed_incr (unsigned by) {
    std::lock_guard<std::mutex> guard (mutex_);
    completed_ += by;
    assert (completed_ <= total_);
    return *this;
}

// total
// ~~~~~
unsigned updater::total () const {
    std::lock_guard<std::mutex> guard (mutex_);
    return total_;
}
updater & updater::total (unsigned value) {
    std::lock_guard<std::mutex> guard (mutex_);
    total_ = value;
    assert (total_ >= completed_);
    return *this;
}
updater & updater::total_incr (unsigned by) {
    std::lock_guard<std::mutex> guard (mutex_);
    total_ += by;
    assert (total_ >= completed_);
    return *this;
}

// have_reliable_estimate
// ~~~~~~~~~~~~~~~~~~~~~~
bool updater::have_reliable_estimate (float const average) const {
    assert (average > 0.0F);
    static auto min_samples = std::size_t{10};

    // There are too few samples, the average rate of progress is too low or average rate of
    // progress is too unpredictable (standard deviation is very large), then we decline to provide
    // an estimate.
    return time_remaining_.samples () > min_samples && average >= 1.0 &&
           time_remaining_.standard_deviation () < average;
}

// time_left
// ~~~~~~~~~
std::ostream & updater::time_left (float const average, std::ostream & os) const {
    assert (average > 0.0F);
    auto const seconds = std::round ((total_ - completed_) / average);
    details::hours_minutes_seconds hms (seconds);
    return os << hms;
}

// update
// ~~~~~~
void updater::update () {
    std::lock_guard<std::mutex> guard (mutex_);
    if (completed_ != old_completed_) {
        bar_.set_value (total_ == 0 ? 0.0F : static_cast<float> (completed_) / total_);

        auto const completed_per_second =
            std::max (static_cast<float> (completed_ - old_completed_), 0.0F);
        float const average = time_remaining_.update (completed_per_second);

        std::ostringstream str;
        str << "  " << completed_ << " of " << total_;
        if (this->have_reliable_estimate (average)) {
            str << " (about ";
            this->time_left (average, str);
            str << " remaining)";
        }

        std::string message = str.str ();
        std::size_t const message_width = std::max (prev_width_, message.length ());
        prev_width_ = message.length ();

        message.append (message_width - message.length (), ' ');
        assert (message.length () == message_width);
        print_cout_flush ('\r', bar_, message);

        old_completed_ = completed_;
    }
}

// run
// ~~~
updater & updater::run () {
    std::lock_guard<std::mutex> guard (mutex_);
    if (timer_.get () == nullptr) {
        old_completed_ = completed_;
        prev_width_ = 0;
        timer_ = setInterval (std::bind (&updater::update, this), std::chrono::milliseconds{1000});
    }
    return *this;
}
// eof progress.js
