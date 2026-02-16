// AnnotationSet.cpp
#include "AnnotationSet.hpp"

// #include "TagManager.hpp"

// Notify TagManager that a tag was added to this object
void AnnotationSet::NotifyAdded(std::string_view tag) noexcept {
    if (mManager) {

        // mManager->OnTagAdded(mOwner, tag);
    }
}

// Notify TagManager that a tag was removed from this object
void AnnotationSet::NotifyRemoved(std::string_view tag) noexcept {
    if (mManager) {

        // mManager->OnTagRemoved(mOwner, tag);
    }
}
