#pragma once

#include <asp/detail/config.hpp>
#include <optional>
#include <string>

#include "detail.hpp"

namespace asp::time {
    class Duration {
    public:
        constexpr inline Duration() : m_seconds(0), m_nanos(0) {}
        constexpr inline Duration(const Duration& other) = default;
        constexpr inline Duration& operator=(const Duration& other) = default;
        constexpr inline Duration(Duration&& other) = default;
        constexpr inline Duration& operator=(Duration&& other) = default;

        constexpr inline Duration(u64 seconds, u32 nanos) : m_seconds(seconds) {
            if (nanos < detail::NANOS_IN_SEC) {
                m_nanos = nanos;
                return;
            }

            seconds += (nanos / detail::NANOS_IN_SEC);
            m_nanos = nanos % detail::NANOS_IN_SEC;
        }

        constexpr static inline Duration fromSecs(u64 secs) { return _unchecked(secs, 0); }
        constexpr static inline std::optional<Duration> fromSecsF32(f32 secs) {
            if (secs < +0.f) {
                return std::nullopt;
            }

            auto whole = detail::floor(secs);
            auto fractional = secs - (u64)secs;

            return Duration(
                static_cast<u64>(whole),
                static_cast<u32>(static_cast<f32>(detail::NANOS_IN_SEC) * fractional)
            );
        }

        constexpr static inline std::optional<Duration> fromSecsF64(f64 secs) {
            if (secs < +0.0) {
                return std::nullopt;
            }

            auto whole = detail::floor(secs);
            auto fractional = secs - (u64)secs;

            return Duration(
                static_cast<u64>(whole),
                static_cast<u32>(static_cast<f64>(detail::NANOS_IN_SEC) * fractional)
            );
        }

        constexpr static inline Duration fromMillis(u64 ms) {
            return _unchecked(detail::millis_to_secs(ms), detail::millis_to_nanos_subsec(ms));
        }
        constexpr static inline Duration fromMicros(u64 micros) {
            return _unchecked(detail::micros_to_secs(micros), detail::micros_to_nanos_subsec(micros));
        }
        constexpr static inline Duration fromNanos(u64 nanos) {
            return _unchecked(detail::nanos_to_secs(nanos), nanos % detail::NANOS_IN_SEC);
        }
        constexpr static inline Duration fromMinutes(u64 mins) { return _unchecked(detail::minutes_to_secs(mins), 0); }
        constexpr static inline Duration fromHours(u64 hours) { return _unchecked(detail::hours_to_secs(hours), 0); }
        constexpr static inline Duration fromDays(u64 days) { return _unchecked(detail::days_to_secs(days), 0); }
        constexpr static inline Duration fromWeeks(u64 weeks) { return _unchecked(detail::weeks_to_secs(weeks), 0); }

        // Getter / converter functions

        constexpr inline bool isZero() const {
            return m_seconds == 0 && m_nanos == 0;
        }

        // Note: recommended not to use unless 128-bit support is enabled.
        constexpr inline u128 micros() const {
            return static_cast<u128>(m_seconds) * static_cast<u128>(detail::MICROS_IN_SEC) + static_cast<u128>(detail::nanos_to_micros(m_nanos));
        }

        // Note: recommended not to use unless 128-bit support is enabled.
        constexpr inline u128 nanos() const {
            return static_cast<u128>(m_seconds) * static_cast<u128>(detail::NANOS_IN_SEC) + static_cast<u128>(m_nanos);
        }

        template <typename As = u64>
        constexpr As millis() const;

        template<> constexpr u64 millis() const {
            return m_seconds * detail::MILLIS_IN_SEC + detail::nanos_to_millis(m_nanos);
        }

        template<> constexpr f32 millis() const {
            return (f32)m_seconds * (f32)detail::MILLIS_IN_SEC + (f32)m_nanos / (f32)detail::NANOS_IN_MILLISEC;
        }

        template<> constexpr f64 millis() const {
            return (f64)m_seconds * (f64)detail::MILLIS_IN_SEC + (f64)m_nanos / (f64)detail::NANOS_IN_MILLISEC;
        }

        template <typename As = u64>
        constexpr As seconds() const;

        template<> u64 seconds() const { return m_seconds; }
        template<> f32 seconds() const {
            return static_cast<f32>(m_seconds) + static_cast<f32>(m_nanos) / static_cast<f32>(detail::NANOS_IN_SEC);
        }
        template<> f64 seconds() const {
            return static_cast<f64>(m_seconds) + static_cast<f64>(m_nanos) / static_cast<f64>(detail::NANOS_IN_SEC);
        }

        constexpr inline u64 minutes() const { return detail::secs_to_minutes(m_seconds); }
        constexpr inline u64 hours() const { return detail::secs_to_hours(m_seconds); }
        constexpr inline u64 days() const { return detail::secs_to_days(m_seconds); }
        constexpr inline u64 weeks() const { return detail::secs_to_weeks(m_seconds); }

        constexpr inline u64 subsecMillis() const { return detail::nanos_to_millis(m_nanos); }
        constexpr inline u64 subsecMicros() const { return detail::nanos_to_micros(m_nanos); }
        constexpr inline u64 subsecNanos() const { return m_nanos; }

        // Return the amount of seconds as a float, including


        // Various operations

        [[nodiscard]] constexpr inline Duration absDiff(const Duration& other) const {
            if (auto res = this->checkedSub(other)) {
                return res.value();
            } else {
                return other.checkedSub(*this).value();
            }
        }

        [[nodiscard]] constexpr inline std::optional<Duration> checkedAdd(const Duration& other) const {
            u64 addedSecs;
            if (!::asp::checkedAdd(addedSecs, m_seconds, other.m_seconds)) {
                return std::nullopt;
            }

            u32 nanos = this->m_nanos + other.m_nanos;
            if (nanos > detail::NANOS_IN_SEC) {
                nanos -= detail::NANOS_IN_SEC;

                u64 temp;
                if (!::asp::checkedAdd(temp, addedSecs, 1ULL)) {
                    return std::nullopt;
                }

                addedSecs = temp;
            }

            return _unchecked(addedSecs, nanos);
        }

        [[nodiscard]] constexpr inline std::optional<Duration> checkedSub(const Duration& other) const {
            u64 subdSecs;
            if (!::asp::checkedSub(subdSecs, m_seconds, other.m_seconds)) {
                return std::nullopt;
            }

            u32 nanos;
            if (m_nanos >= other.m_nanos) {
                nanos = m_nanos - other.m_nanos;
            } else {
                u64 temp;
                if (!::asp::checkedSub(temp, subdSecs, 1ULL)) {
                    return std::nullopt;
                }

                subdSecs = temp;
                nanos = m_nanos + detail::NANOS_IN_SEC - other.m_nanos;
            }

            return _unchecked(subdSecs, nanos);
        }

        [[nodiscard]] constexpr inline std::optional<Duration> checkedMul(u32 by) const {
            auto totalNanos = static_cast<u64>(m_nanos) * static_cast<u64>(by);
            auto extraSecs = detail::nanos_to_secs(totalNanos);
            auto nanos = (totalNanos % detail::NANOS_IN_SEC);

            u64 mulSecs;
            if (!::asp::checkedMul(mulSecs, m_seconds, static_cast<u64>(by))) {
                return std::nullopt;
            }

            u64 addedSecs;
            if (!::asp::checkedAdd(addedSecs, mulSecs, extraSecs)) {
                return std::nullopt;
            }

            return _unchecked(addedSecs, nanos);
        }

        std::string toString(u8 precision) const;

        // Operators

        constexpr inline Duration operator+(const Duration& other) const {
            if (auto res = this->checkedAdd(other)) {
                return res.value();
            } else {
                detail::_throwrt("overflow when adding durations");
            }
        }

        constexpr inline Duration& operator+=(const Duration& other) {
            *this = *this + other;
            return *this;
        }

        constexpr inline Duration operator-(const Duration& other) const {
            if (auto res = this->checkedSub(other)) {
                return res.value();
            } else {
                detail::_throwrt("overflow when subtracting durations");
            }
        }

        constexpr inline Duration& operator-=(const Duration& other) {
            *this = *this - other;
            return *this;
        }

        constexpr inline Duration operator*(u32 val) const {
            if (auto res = this->checkedMul(val)) {
                return res.value();
            } else {
                detail::_throwrt("overflow when multiplying duration by scalar");
            }
        }

        constexpr inline Duration& operator*=(u32 val) {
            *this = *this * val;
            return *this;
        }

    private:
        u64 m_seconds;
        // Invariant: m_nanos always between 0 and `s_to_ns(1)`
        u32 m_nanos;

        constexpr static inline Duration _unchecked(u64 secs, u32 nanos) {
            Duration dur;
            dur.m_seconds = secs;
            dur.m_nanos = nanos;
            return dur;
        }
    };
}