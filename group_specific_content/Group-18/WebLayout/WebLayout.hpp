#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cassert>
#include <emscripten/val.h>
#include "IDomElement.hpp"

using ElementID = std::string;

enum class LayoutType { Free, Horizontal, Vertical, Grid };

class WebLayout : public IDomElement {
public:
	explicit WebLayout(const std::string& rootId = "");
	~WebLayout();

	// IDomElement interface
	void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
	void unmount() override;
	void syncFromModel() override;

	void SetLayoutType(LayoutType t);
	void SetSpacing(int px);

	bool AddElement(const ElementID& id, Alignment align = Alignment::Start);
	bool RemoveElement(const ElementID& id);
	void SetAlignment(const ElementID& id, Alignment a);

	void Apply();
	void Clear();

	const std::string& Id() const override { return id_; }

private:
	emscripten::val root_;
	std::string id_;
	std::vector<ElementID> children_;
	std::unordered_map<ElementID, Alignment> params_;
	LayoutType type_ = LayoutType::Free;
	int spacing_ = 0;

	static int nextIdCounter_;
};

