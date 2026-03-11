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
#include <stdexcept> // Allows the program to throw runtime errors


namespace cse498 {

    class Random {
        private:
            // Constants
            static constexpr int STATE_NUMBER = 4;
            static constexpr int NUM_BITS = 64;
            
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

            uint64_t m_seed;


            ////////////////////////////////////////////////////////////
            ///
            /// Xoshiro generation functions
            /// adapted from the credited source above
            ///

            // keeps an internal state of 4 64 unsigned 64-bit ints
            // changes each time a number is generated
            struct m_Xoshiro256ppState {
                uint64_t s[STATE_NUMBER];
            };

            struct m_Splitmix64State {
                uint64_t s;
            };

            // Ensursed state values are non-zero and well-mixed.
            uint64_t m_Splitmix64(struct m_Splitmix64State &state) {
                uint64_t result = (state.s += GOLDEN_RATIO);
                result = (result ^ (result >> RIGHT_SHIFT1)) * FIRST_MIXING;
                result = (result ^ (result >> RIGHT_SHIFT2)) * SECOND_MIXING;
                return result ^ (result >> RIGHT_SHIFT3);
            }

            // Uses m_seed to generate the state positions
            void m_Xoshiro256ppInit(struct m_Xoshiro256ppState &state) {
                struct m_Splitmix64State sm = {m_seed};

                state.s[0] = m_Splitmix64(sm);
                state.s[1] = m_Splitmix64(sm);
                state.s[2] = m_Splitmix64(sm);
                state.s[3] = m_Splitmix64(sm);
            }

            // Performs a left rotation on x by k bits
            uint64_t m_Rol64(uint64_t x, int k) {
                // Undefined behavior should never trigger due to constant values
                // If this triggers, function was used incorrectly.
                if (k == 0) {
                    throw std::runtime_error("cse498::Random::m_Rol64(): k = 0 is undefined behavior");
                }
                else if (k == 64) {
                    throw std::runtime_error("cse498::Random::m_Rol64(): k = 64 is undefined behavior");
                }

                return (x<<k) | (x >> (NUM_BITS-k));
            }

            // Generates a random number
            uint64_t m_Xoshiro256pp(struct m_Xoshiro256ppState &state) {
                uint64_t *s = state.s;

                // Adds parts 0 and 3 of the state, rotates the sum left by 23 bits
                // then adds part 0 back into the sum
                uint64_t result = m_Rol64(s[0] + s[3], LEFT_SHIFT) + s[0];

                // shifts state 1 by 17 bits
                // helps with state mixing
                uint64_t t = s[1] << MIX_SHIFT;
                
                // Mix all 4 state values
                s[2] ^= s[0];
                s[3] ^= s[1];
                s[1] ^= s[2];
                s[0] ^= s[3];
                s[2] ^= t;
                s[3] = m_Rol64(s[3], NUM_STATE_VALUES);
                return result;
            }

            ///
            ////////////////////////////////////////////////////////////

            
            // Generates a double
            double m_DoubleXoshiro(struct m_Xoshiro256ppState &state) {
                uint64_t r = m_Xoshiro256pp(state);
                return (r >> D_LOWER_11) * (DOUBLE_CONVERSION_FACTOR); // Using the top 53 bits
            }

            // Generates a float
            float m_FloatXoshiro(struct m_Xoshiro256ppState &state) {
                uint64_t r = m_Xoshiro256pp(state);
                return static_cast<float>(r >> F_LOWER_41) * (FLOAT_CONVERSION_FACTOR); // Using the top 23 bits
            }

            struct m_Xoshiro256ppState m_rng;
            bool m_used = false;

            void m_CheckRng() {
                if (m_used == false) {
                    m_Xoshiro256ppInit(m_rng);
                    m_used = true;
                }
            }

        public:
            Random() {
                m_seed =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            }

            // Public getter and setter for the current seed.
            void SetSeed(uint64_t seed) {
                m_seed = seed;
                m_used = false;
            }
            uint64_t GetSeed() const {return m_seed;}

            // Generate and return a random int
            int GetInt(int i_min = 0, int i_max = 100) {
                // Error handling: i_min must be less than i_max
                if (i_min > i_max) {
                    throw std::runtime_error("cse498::Random::GetInt(): i_min must be less than i_max.");
                }
                
                m_CheckRng();

                // generate the bits
                uint64_t r = m_Xoshiro256pp(m_rng);
                // make use of the bits useful for a integer
                int return_i = i_min + static_cast<int>(r % static_cast<uint64_t>(i_max-i_min+1));

                return return_i;
            }

            // Generate and return a random double
            double GetDouble(double d_min = 0.0, double d_max = 100.0){
                // Error handling: d_min must be less than d_max
                if (d_min > d_max) {
                    throw std::runtime_error("cse498::Random::GetDouble(): d_min must be less than d_max.");
                }

                m_CheckRng();

                // for doubles use the 53 bits
                double return_d = d_min + (d_max - d_min) * m_DoubleXoshiro(m_rng);

                return return_d;
            }

            // Generate and return a random float
            float GetFloat(float f_min = 0.0f, float f_max = 100.0f) {
                // Error handling: f_min must be less than f_max
                if (f_min > f_max) {
                    throw std::runtime_error("cse498::Random::GetFloat(): f_min must be less than f_max.");
                }

                m_CheckRng();

                // for floats use the top 23 bits
                float return_f = f_min + (f_max - f_min) * m_FloatXoshiro(m_rng);

                return return_f;
            }

            // Generate and return a random char
            char GetChar(char c_min = 'A', char c_max = 'Z'){
                // Error handling: c_min must be less than c_max
                if (c_min > c_max) {
                    throw std::runtime_error("cse498::Random::GetChar(): c_min must be less than c_max.");
                }

                m_CheckRng();
                
                // Treat chars as ints
                uint64_t r = m_Xoshiro256pp(m_rng);
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
                    throw std::runtime_error("cse498::Random::P(): parameter probability must be between 0 and 1.");
                }

                m_CheckRng();

                // Uses the double generation to determine true/false
                bool return_p = m_DoubleXoshiro(m_rng) < probability;

                return return_p;
            }
    };
}