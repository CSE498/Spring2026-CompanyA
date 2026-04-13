#pragma once
#include "../../../source/Interfaces/WebUI/WebLayout/WebLayout.hpp"

using namespace cse498;

struct SharedWebContext {
    WebLayout root = WebLayout("testing-root");

    SharedWebContext() {
        // add root testing layout to elements can be added to it and show up in the DOM
        GetDocument()["body"].call<void>("appendChild", root.GetElement());
    }
    ~SharedWebContext() {
        root.Clear();
        GetDocument()["body"].call<void>("removeChild", root.GetElement());
    }
};