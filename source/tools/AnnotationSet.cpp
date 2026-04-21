/**
 * @file AnnotationSet.cpp
 * @brief Implementations for AnnotationSet.
 */

#include "AnnotationSet.hpp"

/**
 * @brief Reserved hook: notify TagManager that a tag was added.
 * @param tag Tag that was inserted (currently unused when manager hooks are disabled).
 */
void cse498::AnnotationSet::NotifyAdded([[maybe_unused]] std::string_view tag) noexcept {
    // if (mManager) {

    //      mManager->OnTagAdded(mOwner, tag);
    // }
}

/**
 * @brief Reserved hook: notify TagManager that a tag was removed.
 * @param tag Tag that was removed (currently unused when manager hooks are disabled).
 */
void cse498::AnnotationSet::NotifyRemoved([[maybe_unused]] std::string_view tag) noexcept {
    // if (mManager) {

    //     mManager->OnTagRemoved(mOwner, tag);
    // }
}

/// @brief Assert that @p tag is non-empty.
void cse498::AnnotationSet::AssertValidTag(std::string_view tag) {
    assert(!tag.empty() && "AnnotationSet: tag must not be empty");
}

bool cse498::AnnotationSet::AddTag(std::string_view tag) {
    AssertValidTag(tag);

    std::string owned(tag);

    auto [it, inserted] = mTags.emplace(tag);
    if (!inserted) {
        return false;
    }

    NotifyAdded(*it);
    return true;
}

bool cse498::AnnotationSet::RemoveTag(std::string_view tag) {
    AssertValidTag(tag);

    auto it = mTags.find(std::string(tag));
    if (it == mTags.end()) {
        return false;
    }

    NotifyRemoved(*it);
    mTags.erase(it);

    return true;
}

bool cse498::AnnotationSet::HasTag(std::string_view tag) const {
    AssertValidTag(tag);
    return mTags.find(std::string(tag)) != mTags.end();
}

void cse498::AnnotationSet::Clear() {
    if (!mManager) {
        mTags.clear();
        return;
    }

    for (const auto& t: mTags) {
        NotifyRemoved(t);
    }
    mTags.clear();
}
