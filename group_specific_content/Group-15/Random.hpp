/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * It is apart of Green and White Games (Company A) Group 15's module.
 * @brief A class for random generation
 * @note Status: PROPOSAL
 * Credit notes: This files makes use of the Xoshiro algorithm as described at:
 *               https://en.wikipedia.org/wiki/Xorshift#xoshiro
 **/

#pragma once

#include <stdint.h>
#include <chrono>
#include <stdbool.h>
#include <stdexcept>

// Constants
constexpr int STATE_NUMBER = 4;
constexpr int NUM_BITS = 64;

constexpr int RIGHT_SHIFT1 = 30;
constexpr int RIGHT_SHIFT2 = 27;
constexpr int RIGHT_SHIFT3 = 31;
constexpr uint64_t GOLDEN_RATIO = 0x9E3779B97F4A7C15ULL;
constexpr uint64_t FIRST_MIXING = 0xBF58476D1CE4E5B9;
constexpr uint64_t SECOND_MIXING = 0x94D049BB133111EB;

constexpr int LEFT_SHIFT = 23;
constexpr int MIX_SHIFT = 17;
constexpr int NUM_STATE_VALUES = 45;

constexpr int D_LOWER_11 = 11;
constexpr double DOUBLE_CONVERSION_FACTOR = 1.0 / 9007199254740992.0;
constexpr int F_LOWER_41 = 41;
constexpr float FLOAT_CONVERSION_FACTOR = 1.0f / 8388608.0f;





namespace cse498 {
    class Random {
        private:
            uint64_t m_seed;

            ///
            /// Xoshiro generation functions
            ///

            // keeps an internal state of 4 64 unsigned 64-bit ints
            // changes each time a number is generated
            struct xoshiro256pp_state {
                uint64_t s[STATE_NUMBER];
            };

            struct splitmix64_state {
                uint64_t s;
            };

            // Ensursed state values are non-zero and well-mixed.
            uint64_t splitmix64(struct splitmix64_state *state) {
                uint64_t result = (state->s += GOLDEN_RATIO);
                result = (result ^ (result >> RIGHT_SHIFT1)) * FIRST_MIXING;
                result = (result ^ (result >> RIGHT_SHIFT2)) * SECOND_MIXING;
                return result ^ (result >> RIGHT_SHIFT3);
            }

            // Uses m_seed to generate the state positions
            void xoshiro356pp_init(struct xoshiro256pp_state *state, uint64_t seed) {
                struct splitmix64_state sm = {seed};

                state->s[0] = splitmix64(&sm);
                state->s[1] = splitmix64(&sm);
                state->s[2] = splitmix64(&sm);
                state->s[3] = splitmix64(&sm);
            }

            // Performs a left rotation on x by k bits
            uint64_t rol64(uint64_t x, int k) {
                return (x<<k) | (x >> (NUM_BITS-k));
            }

            // Generates a random number
            uint64_t xoshiro256pp(struct xoshiro256pp_state * state) {
                uint64_t *s = state->s;

                // Adds parts 0 and 3 of the state, rotates the sum left by 23 bits
                // then adds part 0 back into the sum
                uint64_t result = rol64(s[0] + s[3], LEFT_SHIFT) + s[0];

                // shifts state 1 by 17 bits
                // helps with state mixing
                uint64_t t = s[1] << MIX_SHIFT;
                
                // Mix all 4 state values
                s[2] ^= s[0];
                s[3] ^= s[1];
                s[1] ^= s[2];
                s[0] ^= s[3];
                s[2] ^= t;
                s[3] = rol64(s[3], NUM_STATE_VALUES);
                return result;
            }

            // Generates a double
            double double_xoshiro(struct xoshiro256pp_state *state) {
                uint64_t r = xoshiro256pp(state);
                return (r >> D_LOWER_11) * (DOUBLE_CONVERSION_FACTOR); // Using the top 53 bits
            }

            // Generates a float
            float float_xoshiro(struct xoshiro256pp_state *state) {
                uint64_t r = xoshiro256pp(state);
                return static_cast<float>(r >> F_LOWER_41) * (FLOAT_CONVERSION_FACTOR); // Using the top 23 bits
            }

            // Function to reset the seed based on the current time
            void reset_seed() {
                m_seed =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            }

            struct xoshiro256pp_state m_rng;
            bool m_used = false;

        protected:

        public:
            Random() {
                reset_seed();
            }
            virtual ~Random() = default;

            // Public setter for the current seed.
            // Must be re-set after every generation, if you would like to keep the same seed
            void SetSeed(uint64_t seed) {m_seed = seed;}

            // Generate and return a random int
            int GetInt(int i_min = 0, int i_max = 100) {
                // Error handling: i_min must be less than i_max
                if (i_min > i_max) {
                    throw std::runtime_error("cse498::Random::GetInt(): i_min must be less than i_max.");
                }

                if (m_used == false) {
                    xoshiro356pp_init(&m_rng, m_seed);
                    m_used = true;
                }

                // generate the range
                uint64_t r = xoshiro256pp(&m_rng);
                int return_i = i_min + static_cast<int>(r % static_cast<uint64_t>(i_max-i_min+1));

                return return_i;
            }

            // Generate and return a random double
            double GetDouble(double d_min = 0.0, double d_max = 100.0){
                // Error handling: d_min must be less than d_max
                if (d_min > d_max) {
                    throw std::runtime_error("cse498::Random::GetDouble(): d_min must be less than d_max.");
                }

                if (m_used == false) {
                    xoshiro356pp_init(&m_rng, m_seed);
                    m_used = true;
                }

                // for doubles use the 53 bits
                double return_d = d_min + (d_max - d_min) * double_xoshiro(&m_rng);

                return return_d;
            }

            // Generate and return a random float
            float GetFloat(float f_min = 0.0f, float f_max = 100.0f) {
                // Error handling: f_min must be less than f_max
                if (f_min > f_max) {
                    throw std::runtime_error("cse498::Random::GetFloat(): f_min must be less than f_max.");
                }

                if (m_used == false) {
                    xoshiro356pp_init(&m_rng, m_seed);
                    m_used = true;
                }

                // for floats use the top 23 bits
                float return_f = f_min + (f_max - f_min) * float_xoshiro(&m_rng);

                return return_f;
            }

            // Generate and return a random char
            char GetChar(char c_min = 'A', char c_max = 'Z'){
                // Error handling: c_min must be less than c_max
                if (c_min > c_max) {
                    throw std::runtime_error("cse498::Random::GetChar(): c_min must be less than c_max.");
                }

                if (m_used == false) {
                    xoshiro356pp_init(&m_rng, m_seed);
                    m_used = true;
                }
                
                // Treat chars as ints
                uint64_t r = xoshiro256pp(&m_rng);
                char return_c = static_cast<char>(c_min + (r % static_cast<uint64_t>(c_max - c_min + 1)));

                return return_c;
            }

            // Generate and return a random bool
            bool GetBool(){
                //Generate between 0 and 1 using an int
                int i = GetInt(0,1);

                if (i == 0){
                    return false;
                }
                return true;
            }

            // Generates based off of a given probability, and returns a bool
            bool P(double probability = 0.5){
                // Error handling: probability must be between 0 and 1
                if (0 > probability || 1 < probability) {
                    throw std::runtime_error("cse498::Random::GetP(): probability must be between 0 and 1.");
                }

                if (m_used == false) {
                    xoshiro356pp_init(&m_rng, m_seed);
                    m_used = true;
                }

                // Uses the double generation to determine true/false
                bool return_p = double_xoshiro(&m_rng) < probability;

                return return_p;
            }
    };
}