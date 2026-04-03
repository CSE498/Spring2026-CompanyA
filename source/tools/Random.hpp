/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * It is apart of Green and White Games (Company A) Group 15's module.
 * @brief A class for random generation
 * @note Status: PROPOSAL
 * Credit notes: This files makes use of the Xoshiro algorithm as described at:
 *               https://en.wikipedia.org/wiki/Xorshift#xoshiro
 **/

#pragma once

#include <cstdint>  // This defined the size of uint64_t
#include <chrono>    // Used to get the current time off of the computer
#include <expected>  // Allows the program to have errors for input
#include <array>     // Allows the program to use std::array
#include <concepts>  // Allows for greater template functionality


namespace cse498 {

    class Random {
        private:
            // Constants
            static constexpr int STATE_NUMBER = 4; // The number of states being used to generate values
            static constexpr int NUM_BITS = 64;    // The number of bits per state

            static constexpr int RIGHT_SHIFT1 = 30;
            static constexpr int RIGHT_SHIFT2 = 27;
            static constexpr int RIGHT_SHIFT3 = 31;
            static constexpr uint64_t GOLDEN_RATIO = 0x9E3779B97F4A7C15ULL;
            static constexpr uint64_t FIRST_MIXING = 0xBF58476D1CE4E5B9;
            static constexpr uint64_t SECOND_MIXING = 0x94D049BB133111EB;
            
            static constexpr int LEFT_SHIFT = 23;
            static constexpr int MIX_SHIFT = 17;
            static constexpr int NUM_STATE_VALUES = 45;
            
            static constexpr int D_LOWER_11 = 11;
            static constexpr double DOUBLE_CONVERSION_FACTOR = 1.0 / 9007199254740992.0;
            static constexpr int F_LOWER_41 = 41;
            static constexpr float FLOAT_CONVERSION_FACTOR = 1.0f / 8388608.0f;

            uint64_t m_seed;  // The seed used for random generation


            ////////////////////////////////////////////////////////////
            ///
            /// Xoshiro generation functions
            /// adapted from the credited source above
            ///

            // keeps an internal state of 4 64 unsigned 64-bit ints
            // changes each time a number is generated
            struct Xoshiro256ppState {
                std::array<uint64_t, STATE_NUMBER> state;
            };

            struct Splitmix64State {
                uint64_t state;
            };

            /// @brief Ensures state values are non-zero and well-mixed.
            /// @param The splitmix state being mixed
            /// @return A well-mixed, non-zero uint64_t value
            uint64_t Splitmix64(struct Splitmix64State &state) {
                uint64_t result = (state.state += GOLDEN_RATIO);
                result = (result ^ (result >> RIGHT_SHIFT1)) * FIRST_MIXING;
                result = (result ^ (result >> RIGHT_SHIFT2)) * SECOND_MIXING;
                return result ^ (result >> RIGHT_SHIFT3);
            }

            /// @brief Uses m_seed to generate the state positions
            /// @param The xoshiro state being initialize
            void Xoshiro256ppInit(struct Xoshiro256ppState &state) {
                struct Splitmix64State sm = {m_seed};

                for (auto &value : state.state) {
                    value = Splitmix64(sm);
                }
            }

            /// @brief Performs a left rotation on x by k bits
            /// @param x and k, x is the value being rotated and k is how much it is rotated by
            /// @return A rotated x value
            uint64_t RotateLeft64(uint64_t x, int k) {
                return (x<<k) | (x >> (NUM_BITS-k));
            }

            /// @brief Generates a random number
            /// @param the xoshiro state being used to generated the number
            /// @return a randomly generated uint64_t value
            uint64_t Xoshiro256pp(struct Xoshiro256ppState &state) {
                auto &new_state = state.state;

                // Adds parts 0 and 3 of the state, rotates the sum left by 23 bits
                // then adds part 0 back into the sum
                uint64_t result = RotateLeft64(new_state[0] + new_state[3], LEFT_SHIFT) + new_state[0];

                // shifts state 1 by 17 bits
                // helps with state mixing
                uint64_t temp_state = new_state[1] << MIX_SHIFT;
                
                // Mix all 4 state values
                new_state[2] ^= new_state[0];
                new_state[3] ^= new_state[1];
                new_state[1] ^= new_state[2];
                new_state[0] ^= new_state[3];
                new_state[2] ^= temp_state;
                new_state[3] = RotateLeft64(new_state[3], NUM_STATE_VALUES);
                return result;
            }

            ///
            ////////////////////////////////////////////////////////////


            /// @brief Generates a double
            /// @param the xoshiro state being used to generated the number
            /// @return a randomly generated double
            double DoubleXoshiro(struct Xoshiro256ppState &state) {
                uint64_t random_bits = Xoshiro256pp(state);
                return (random_bits >> D_LOWER_11) * (DOUBLE_CONVERSION_FACTOR); // Using the top 53 bits
            }

            struct Xoshiro256ppState m_rng;
            bool m_used = false;

            /// @brief Checks if the rng has been initialize or not
            void CheckRng() {
                if (!m_used) {
                    Xoshiro256ppInit(m_rng);
                    m_used = true;
                }
            }

        public:
            /// @brief The constructor for a Random object
            Random() {
                m_seed =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            }
            
            /// @brief An overloaded contructor to take an optional seed
            /// @param seed - a optional parameter to set the seed of the generator
            Random(uint64_t seed) {
                m_seed =  seed;
            }

            /// @brief Public setter for the current seed.
            /// @param the desired seed
            void SetSeed(uint64_t seed) {
                m_seed = seed;
                m_used = false;
            }
            /// @brief Public getter for the current seed
            /// @return the current seed
            uint64_t GetSeed() const {return m_seed;}


            /// @brief Templated function to generate and return values for integral values
            /// @param The range of values(inclusive) to generate a number between. min must be <= max.
            /// @return A random value in range of the specified type
            template <std::integral T>
            std::expected<T, std::string> GetValue(T min, T max) {
                if (min > max){
                    return std::unexpected("cse498::Random::GetValue(): min must be less than or equal to max.");
                }

                CheckRng();

                // generate the bits
                uint64_t r = Xoshiro256pp(m_rng);
                
                // Handle int type values: ints, bools, chars
                //return min + static_cast<T> (r % static_cast<uint64_t>(max-min+1));
                uint64_t range = static_cast<uint64_t>(max) - static_cast<uint64_t>(min) + 1;
                __uint128_t product = static_cast<__uint128_t>(r) * range;
                uint64_t scaled = static_cast<uint64_t>(product >> 64);
                return min + static_cast<T>(scaled);
            }

            /// @brief Templated function to generate and return values for floating point values
            /// @param The range of values(inclusive) to generate a number between. min must be <= max.
            /// @return A random value in range of the specified type
            template <std::floating_point T>
            std::expected<T, std::string> GetValue(T min, T max) {
                if (min > max){
                    return std::unexpected("cse498::Random::GetValue(): min must be less than or equal to max.");
                }

                CheckRng();

                // generate the bits
                uint64_t r = Xoshiro256pp(m_rng);
                
                // Handle decimal type values: doubles, floats
                double decimal_value = static_cast<double>(r)/static_cast<double>(UINT64_MAX);
                return static_cast<T>(min + decimal_value * (max-min));
            }

            /// @brief Generates based off of a given probability, and returns a bool
            /// @param the desired probaility of a true value
            /// @return a weighted generated bool
            [[nodiscard]] std::expected<bool, std::string> P(double probability = 0.5){
                // Error handling: probability must be between 0 and 1
                if (0 > probability){
                    return std::unexpected("cse498::Random::P(): probability must be greater than or equal to 0");
                }
                if (1 < probability){
                    return std::unexpected("cse498::Random::P(): probability must be less than or equal to 1");
                }

                CheckRng();

                // Uses the double generation to determine true/false
                bool return_p = DoubleXoshiro(m_rng) < probability;

                return return_p;
            }
    };
}