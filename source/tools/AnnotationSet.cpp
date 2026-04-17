// AnnotationSet.cpp
#include "AnnotationSet.hpp"

// #include "TagManager.hpp"

// Notify TagManager that a tag was added to this object
void cse498::AnnotationSet::NotifyAdded([[maybe_unused]] std::string_view tag) noexcept {
    // if (mManager) {

    //      mManager->OnTagAdded(mOwner, tag);
    // }
}

// Notify TagManager that a tag was removed from this object
void cse498::AnnotationSet::NotifyRemoved([[maybe_unused]] std::string_view tag) noexcept {
    // if (mManager) {

    //     mManager->OnTagRemoved(mOwner, tag);
    // }
}

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

// Removes all tags
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
