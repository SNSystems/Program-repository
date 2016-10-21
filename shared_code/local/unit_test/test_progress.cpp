#include "progress.hpp"

#include <cmath>
#include <deque>
#include <numeric>

#include <gmock/gmock.h>

namespace {
    class ProgressStat : public ::testing::Test {
    protected:
        // A brute-force implementation of mean.
        template <typename Iterator>
        float mean (Iterator first, Iterator last) {
            static_assert (
                std::is_same<typename std::iterator_traits<Iterator>::value_type, float>::value,
                "must iterate over float");
            auto const total = std::accumulate (first, last, 0.F);
            auto const samples = std::distance (first, last);
            return total / samples;
        }

        // A brute-force implementation of variance.
        template <typename Iterator>
        float variance (Iterator first, Iterator last) {
            static_assert (
                std::is_same<typename std::iterator_traits<Iterator>::value_type, float>::value,
                "must iterate over float");
            auto const xbar = mean (first, last);
            auto const total = std::accumulate (first, last, 0.F, [xbar](float acc, float v) {
                auto const d = v - xbar;
                return acc + d * d;
            });
            auto const samples = std::distance (first, last);
            return total / samples;
        }

        template <typename Iterator>
        float stddev (Iterator first, Iterator last) {
            return std::sqrt (variance (first, last));
        }
    };
}

TEST_F (ProgressStat, PushPop) {
    std::deque<float> values{10.F, 20.F, 15.F, 7.F, 17.F};
    details::time_remaining_calculator::stats est;
    for (auto v : values) {
        est.push (v);
    }
    EXPECT_EQ (5U, est.samples ());
    EXPECT_FLOAT_EQ (mean (std::begin (values), std::end (values)), est.mean ());
    EXPECT_FLOAT_EQ (stddev (std::begin (values), std::end (values)), est.stddev ());

    est.push (21.F);
    values.push_back (21.F);
    ASSERT_FLOAT_EQ (10.F, values.front ());
    est.pop (values.front ());
    values.pop_front ();
    EXPECT_EQ (5U, est.samples ());
    EXPECT_FLOAT_EQ (mean (std::begin (values), std::end (values)), est.mean ());
    EXPECT_FLOAT_EQ (stddev (std::begin (values), std::end (values)), est.stddev ());

    est.push (19.F);
    values.push_back (19.F);
    ASSERT_FLOAT_EQ (20.F, values.front ());
    est.pop (values.front ());
    values.pop_front ();
    EXPECT_EQ (5U, est.samples ());
    EXPECT_FLOAT_EQ (mean (std::begin (values), std::end (values)), est.mean ());
    EXPECT_FLOAT_EQ (stddev (std::begin (values), std::end (values)), est.stddev ());
}



TEST (ProgressHMS, Zero) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (0);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (0U, hms.minutes ());
    EXPECT_EQ (0U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("0 seconds"));
}
TEST (ProgressHMS, One) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (1);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (0U, hms.minutes ());
    EXPECT_EQ (1U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("1 second"));
}
TEST (ProgressHMS, TwoSeconds) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (2);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (0U, hms.minutes ());
    EXPECT_EQ (2U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("2 seconds"));
}
TEST (ProgressHMS, 59Seconds) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (59);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (0U, hms.minutes ());
    EXPECT_EQ (59U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("59 seconds"));
}
TEST (ProgressHMS, 60Seconds) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (60);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (1U, hms.minutes ());
    EXPECT_EQ (0U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("1 minute"));
}
TEST (ProgressHMS, 61Seconds) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (61);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (1U, hms.minutes ());
    EXPECT_EQ (1U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("1 minute, 1 second"));
}
TEST (ProgressHMS, 1Minute59Seconds) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (1 * 60 + 59);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (1U, hms.minutes ());
    EXPECT_EQ (59U, hms.seconds ());
    // EXPECT_THAT (hms.to_string (), StrEq ("1 minute, 59 seconds"));
}
TEST (ProgressHMS, 2Minutes) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (2 * 60);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (2U, hms.minutes ());
    EXPECT_EQ (0U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("2 minutes"));
}
TEST (ProgressHMS, 2Minutes1Second) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (2 * 60 + 1);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (2U, hms.minutes ());
    EXPECT_EQ (0U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("2 minutes"));
}
TEST (ProgressHMS, 2Minutes31Seconds) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (2 * 60 + 31);
    EXPECT_EQ (0U, hms.hours ());
    EXPECT_EQ (3U, hms.minutes ());
    EXPECT_EQ (0U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("3 minutes"));
}
TEST (ProgressHMS, 59Minutes59Seconds) {
    using ::testing::StrEq;

    details::hours_minutes_seconds hms (60 * 60 - 1);
    EXPECT_EQ (1U, hms.hours ());
    EXPECT_EQ (0U, hms.minutes ());
    EXPECT_EQ (0U, hms.seconds ());
    EXPECT_THAT (hms.to_string (), StrEq ("1 hour"));
}


TEST (ProgressTRC, InitialState) {
    details::time_remaining_calculator trc{2};
    EXPECT_FLOAT_EQ (0.0F, trc.mean ());
    EXPECT_FLOAT_EQ (0.0F, trc.standard_deviation ());
    EXPECT_EQ (0U, trc.samples ());
}

TEST (ProgressTRC, OneUpdate) {
    details::time_remaining_calculator trc{2};
    float mean = trc.update (10.0F);
    EXPECT_FLOAT_EQ (10.0F, mean);
    EXPECT_FLOAT_EQ (10.0F, trc.mean ());
    EXPECT_FLOAT_EQ (0.0F, trc.standard_deviation ());
    EXPECT_EQ (1U, trc.samples ());
}

TEST (ProgressTRC, TwoUpdates) {
    details::time_remaining_calculator trc{2};
    trc.update (10.0F);
    float mean = trc.update (20.0F);
    EXPECT_FLOAT_EQ (15.0F, mean);
    EXPECT_FLOAT_EQ (15.0F, trc.mean ());
    EXPECT_FLOAT_EQ (5.F, trc.standard_deviation ());
    EXPECT_EQ (2U, trc.samples ());
}
TEST (ProgressTRC, ThreeUpdates) {
    details::time_remaining_calculator trc{2};
    trc.update (10.0F);
    trc.update (20.0F);
    float mean = trc.update (30.0F);
    EXPECT_FLOAT_EQ (25.0F, mean);
    EXPECT_FLOAT_EQ (mean, trc.mean ());
    EXPECT_FLOAT_EQ (5.0F, trc.standard_deviation ());
    EXPECT_EQ (2U, trc.samples ());
}
// eof test_progress.cpp
