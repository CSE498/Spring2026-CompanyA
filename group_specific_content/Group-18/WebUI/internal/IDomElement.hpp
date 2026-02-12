#pragma once

#include <string>
enum class Alignment { Start, Center, End, Stretch };

class WebLayout; // forward

class IDomElement {
public:
    virtual ~IDomElement() = default;
    virtual void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) = 0;
    virtual void unmount() = 0;
    virtual void syncFromModel() = 0;
    virtual const std::string& Id() const = 0;
};
