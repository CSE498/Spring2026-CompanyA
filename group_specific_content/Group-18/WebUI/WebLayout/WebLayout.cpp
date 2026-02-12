#include "WebLayout.hpp"
#include <sstream>

using emscripten::val;

int WebLayout::nextIdCounter_ = 1;

static std::string px(int v) {
	if (v < 0) return std::string();
	std::ostringstream ss; ss << v << "px"; return ss.str();
}

WebLayout::WebLayout(const std::string& rootId) {
	val document = val::global("document");
	if (!rootId.empty()) {
		val found = document.call<val>("getElementById", rootId);
		if (!found.isNull() && !found.isUndefined()) {
			root_ = found;
		} else {
			// create a container div with the requested id and append to body
			root_ = document.call<val>("createElement", std::string("div"));
			root_.set("id", rootId);
			document["body"].call<void>("appendChild", root_);
		}
		id_ = rootId;
		val style = root_["style"];
		style.set("boxSizing", std::string("border-box"));
		return;
	}

	// no id passed: create a container div and append to body with generated id
	id_ = "weblayout-" + std::to_string(nextIdCounter_++);
	root_ = document.call<val>("createElement", std::string("div"));
	root_.set("id", id_);
	val style = root_["style"];
	style.set("boxSizing", std::string("border-box"));
	document["body"].call<void>("appendChild", root_);
}

WebLayout::~WebLayout() {
	// remove the root element from DOM if present
	val parent = root_["parentNode"];
	if (!parent.isNull() && !parent.isUndefined()) {
		parent.call<void>("removeChild", root_);
	}
}

void WebLayout::SetLayoutType(LayoutType t) {
	type_ = t;
}

void WebLayout::SetSpacing(int pxv) {
	assert(pxv >= 0);
	spacing_ = pxv;
}

bool WebLayout::AddElement(const ElementID& id, Alignment align) {
	assert(!id.empty());

	// avoid duplicates
	for (auto &c : children_) if (c == id) return true;

	// try finding element in document
	val document = val::global("document");
	val el = document.call<val>("getElementById", id);
	if (el.isNull() || el.isUndefined()) {
		return false;
	}

	// append if not already in our root
	root_.call<void>("appendChild", el);

	children_.push_back(id);
	params_[id] = align;
	return true;
}

bool WebLayout::RemoveElement(const ElementID& id) {
	assert(!id.empty());
	auto it = std::find(children_.begin(), children_.end(), id);
	if (it == children_.end()) return false;
	children_.erase(it);
	params_.erase(id);

	val document = val::global("document");
	val el = document.call<val>("getElementById", id);
	if (!el.isNull() && !el.isUndefined()) {
		// detach from root if parented
		val parent = el["parentNode"];
		if (!parent.isNull() && !parent.isUndefined()) {
			parent.call<void>("removeChild", el);
		}
	}
	return true;
}

void WebLayout::SetAlignment(const ElementID& id, Alignment a) {
	assert(!id.empty());
	auto it = params_.find(id);
	if (it == params_.end()) return; // user-level failure tolerated
	it->second = a;
}

// IDomElement implementations
void WebLayout::mountToLayout(WebLayout& parent, Alignment align) {
	// register this layout element with parent and ensure DOM parent-child relationship
	parent.AddElement(id_, align);
}

void WebLayout::unmount() {
	val parent = root_["parentNode"];
	if (!parent.isNull() && !parent.isUndefined()) {
		parent.call<void>("removeChild", root_);
	}
}

void WebLayout::syncFromModel() {
	// default sync behavior: re-apply layout to reflect params/state
	Apply();
}

void WebLayout::Apply() {
	val style = root_["style"];
	switch (type_) {
		case LayoutType::Free:
			style.set("position", std::string("relative"));
			style.set("display", std::string("block"));
			break;
		case LayoutType::Horizontal:
			style.set("display", std::string("flex"));
			style.set("flexDirection", std::string("row"));
			style.set("alignItems", std::string("stretch"));
			style.set("gap", px(spacing_));
			break;
		case LayoutType::Vertical:
			style.set("display", std::string("flex"));
			style.set("flexDirection", std::string("column"));
			style.set("alignItems", std::string("stretch"));
			style.set("gap", px(spacing_));
			break;
		case LayoutType::Grid:
			style.set("display", std::string("grid"));
			style.set("gap", px(spacing_));
			break;
	}

	val document = val::global("document");
	// ensure ordering and apply per-child styles
	for (const auto &id : children_) {
		val el = document.call<val>("getElementById", id);
		if (el.isNull() || el.isUndefined()) continue;

		// append in order
		root_.call<void>("appendChild", el);

		// apply alignment only; sizing is handled by element itself
		auto it = params_.find(id);
		if (it != params_.end()) {
			Alignment a = it->second;
			val est = el["style"];
			std::string alignStr;
			switch (a) {
				case Alignment::Start: alignStr = "flex-start"; break;
				case Alignment::Center: alignStr = "center"; break;
				case Alignment::End: alignStr = "flex-end"; break;
				case Alignment::Stretch: alignStr = "stretch"; break;
			}
			est.set("alignSelf", alignStr);
		}
	}
}

void WebLayout::Clear() {
	// remove children from DOM by clearing innerHTML
	root_.set("innerHTML", std::string(""));
	children_.clear();
	params_.clear();
}
