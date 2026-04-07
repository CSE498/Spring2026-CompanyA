/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Compile-time verification for RobinHoodMap.
 * @note Status: PROPOSAL
 *
 * RobinHoodMap is a template class, so the full implementation lives in the
 * header.  This file exists to verify that the template instantiates correctly
 * at compile time.
 *
 *
 * not completely sure if this is needed, but ill keep it for now.
 **/


#include "RobinHoodMap.hpp"

// example usage on how this works:

// Verify the template compiles for a basic type combination.
static_assert(sizeof(cse498::RobinHoodMap<int, int>) > 0, "RobinHoodMap<int,int> must be instantiable");
