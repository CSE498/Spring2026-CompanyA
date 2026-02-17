#define CATCH_CONFIG_MAIN
#include "../../../../third-party/Catch/single_include/catch2/catch.hpp"

#include "WebButton.hpp"
#include <string>
#include <type_traits>

class WebLayout {
 public:
  bool AddElement(const std::string&, Alignment = Alignment::Start) { return true; }
};

#define CHECK_MSG(cond, msg) \
  do { INFO(msg); CHECK(cond); } while(0)

TEST_CASE("Constructor sets label", "[WebButton]") {
  WebButton btn("Click Me");
  CHECK_MSG(btn.GetLabel() == "Click Me", "label mismatch");
  CHECK_MSG(btn.IsEnabled() == true, "default should be enabled");
  CHECK_MSG(btn.IsVisible() == true, "default should be visible");
  CHECK_MSG(btn.GetWidth() == 0, "default width should be 0");
  CHECK_MSG(btn.GetHeight() == 0, "default height should be 0");
  CHECK_MSG(!btn.Id().empty(), "id should not be empty");
}

TEST_CASE("Constructor default label is empty", "[WebButton]") {
  WebButton btn;
  CHECK_MSG(btn.GetLabel().empty(), "default label should be empty");
}

TEST_CASE("SetLabel and GetLabel", "[WebButton]") {
  WebButton btn("old");
  btn.SetLabel("new");
  CHECK_MSG(btn.GetLabel() == "new", "label not updated");
}

TEST_CASE("SetLabel to empty string", "[WebButton]") {
  WebButton btn("something");
  btn.SetLabel("");
  CHECK_MSG(btn.GetLabel().empty(), "label should be empty");
}

TEST_CASE("SetCallback and Click invokes callback", "[WebButton]") {
  WebButton btn("test");
  int count = 0;
  btn.SetCallback([&count]() { ++count; });
  btn.Click();
  CHECK_MSG(count == 1, "callback should be invoked once");
  btn.Click();
  CHECK_MSG(count == 2, "callback should be invoked twice");
}

TEST_CASE("Click without callback does not crash", "[WebButton]") {
  WebButton btn("test");
  REQUIRE_NOTHROW(btn.Click());
}

TEST_CASE("Click when disabled does not invoke callback", "[WebButton]") {
  WebButton btn("test");
  int count = 0;
  btn.SetCallback([&count]() { ++count; });
  btn.Disable();
  btn.Click();
  CHECK_MSG(count == 0, "callback should not be invoked when disabled");
}

TEST_CASE("Enable Disable IsEnabled", "[WebButton]") {
  WebButton btn("test");
  CHECK_MSG(btn.IsEnabled() == true, "default should be enabled");
  btn.Disable();
  CHECK_MSG(btn.IsEnabled() == false, "should be disabled");
  btn.Enable();
  CHECK_MSG(btn.IsEnabled() == true, "should be enabled again");
}

TEST_CASE("Show Hide IsVisible", "[WebButton]") {
  WebButton btn("test");
  CHECK_MSG(btn.IsVisible() == true, "default should be visible");
  btn.Hide();
  CHECK_MSG(btn.IsVisible() == false, "should be hidden");
  btn.Show();
  CHECK_MSG(btn.IsVisible() == true, "should be visible again");
}

TEST_CASE("Visibility toggle multiple times", "[WebButton]") {
  WebButton btn("test");
  for (int i = 0; i < 5; ++i) {
    btn.Hide();
    CHECK_MSG(btn.IsVisible() == false, "should be hidden");
    btn.Show();
    CHECK_MSG(btn.IsVisible() == true, "should be visible");
  }
}

TEST_CASE("SetSize sets width and height", "[WebButton]") {
  WebButton btn("test");
  btn.SetSize(150, 40);
  CHECK_MSG(btn.GetWidth() == 150, "width mismatch");
  CHECK_MSG(btn.GetHeight() == 40, "height mismatch");
}

TEST_CASE("SetSize with zero dimensions", "[WebButton]") {
  WebButton btn("test");
  btn.SetSize(0, 0);
  CHECK_MSG(btn.GetWidth() == 0, "width should be 0");
  CHECK_MSG(btn.GetHeight() == 0, "height should be 0");
}

TEST_CASE("SetBackgroundColor does not crash", "[WebButton]") {
  WebButton btn("test");
  REQUIRE_NOTHROW(btn.SetBackgroundColor("#4CAF50"));
  REQUIRE_NOTHROW(btn.SetBackgroundColor("rgb(255, 0, 0)"));
  REQUIRE_NOTHROW(btn.SetBackgroundColor("transparent"));
}

TEST_CASE("SetTextColor does not crash", "[WebButton]") {
  WebButton btn("test");
  REQUIRE_NOTHROW(btn.SetTextColor("white"));
  REQUIRE_NOTHROW(btn.SetTextColor("#000000"));
  REQUIRE_NOTHROW(btn.SetTextColor("rgb(0, 128, 255)"));
}

TEST_CASE("Id is unique per instance", "[WebButton]") {
  WebButton a("a");
  WebButton b("b");
  WebButton c("c");
  CHECK_MSG(a.Id() != b.Id(), "a and b should have different ids");
  CHECK_MSG(b.Id() != c.Id(), "b and c should have different ids");
  CHECK_MSG(a.Id() != c.Id(), "a and c should have different ids");
}

TEST_CASE("Id has expected prefix", "[WebButton]") {
  WebButton btn("test");
  std::string id = btn.Id();
  CHECK_MSG(id.rfind("webbutton-", 0) == 0, "id should start with 'webbutton-'");
}

TEST_CASE("syncFromModel does not crash and preserves state", "[WebButton]") {
  WebButton btn("sync");
  btn.SetSize(100, 50);
  btn.SetBackgroundColor("#FF0000");
  btn.SetTextColor("white");
  btn.Disable();
  btn.Hide();
  btn.syncFromModel();
  CHECK_MSG(btn.GetLabel() == "sync", "label should remain");
  CHECK_MSG(btn.GetWidth() == 100, "width should remain");
  CHECK_MSG(btn.GetHeight() == 50, "height should remain");
  CHECK_MSG(btn.IsEnabled() == false, "should remain disabled");
  CHECK_MSG(btn.IsVisible() == false, "should remain hidden");
}

TEST_CASE("Move constructor transfers state", "[WebButton]") {
  WebButton original("move me");
  original.SetSize(200, 60);
  original.Disable();
  original.Hide();
  std::string original_id = original.Id();
  int count = 0;
  original.SetCallback([&count]() { ++count; });

  WebButton moved(std::move(original));
  CHECK_MSG(moved.GetLabel() == "move me", "label should transfer");
  CHECK_MSG(moved.GetWidth() == 200, "width should transfer");
  CHECK_MSG(moved.GetHeight() == 60, "height should transfer");
  CHECK_MSG(moved.IsEnabled() == false, "enabled state should transfer");
  CHECK_MSG(moved.IsVisible() == false, "visibility should transfer");
  CHECK_MSG(moved.Id() == original_id, "id should transfer");
  moved.Enable();
  moved.Click();
  CHECK_MSG(count == 1, "callback should transfer and work");

  CHECK_MSG(original.GetWidth() == 0, "original width should be 0");
  CHECK_MSG(original.GetHeight() == 0, "original height should be 0");
  CHECK_MSG(original.IsEnabled() == false, "original should be disabled");
  CHECK_MSG(original.IsVisible() == false, "original should be hidden");
}

TEST_CASE("Move assignment transfers state", "[WebButton]") {
  WebButton src("source");
  src.SetSize(80, 30);
  src.SetBackgroundColor("#00FF00");
  std::string src_id = src.Id();

  WebButton dest("dest");
  dest = std::move(src);
  CHECK_MSG(dest.GetLabel() == "source", "label should transfer");
  CHECK_MSG(dest.GetWidth() == 80, "width should transfer");
  CHECK_MSG(dest.GetHeight() == 30, "height should transfer");
  CHECK_MSG(dest.Id() == src_id, "id should transfer");
  CHECK_MSG(src.GetWidth() == 0, "src width should be 0");
  CHECK_MSG(src.GetHeight() == 0, "src height should be 0");
  CHECK_MSG(src.IsEnabled() == false, "src should be disabled");
}

TEST_CASE("Callback replacement works", "[WebButton]") {
  WebButton btn("test");
  int first_count = 0;
  int second_count = 0;

  btn.SetCallback([&first_count]() { ++first_count; });
  btn.Click();
  CHECK_MSG(first_count == 1, "first callback should be called");

  btn.SetCallback([&second_count]() { ++second_count; });
  btn.Click();
  CHECK_MSG(first_count == 1, "first callback should still be 1");
  CHECK_MSG(second_count == 1, "second callback should be called");
}

TEST_CASE("Disable then Enable then Click works", "[WebButton]") {
  WebButton btn("test");
  int count = 0;
  btn.SetCallback([&count]() { ++count; });
  btn.Disable();
  btn.Click();
  CHECK_MSG(count == 0, "should not fire when disabled");
  btn.Enable();
  btn.Click();
  CHECK_MSG(count == 1, "should fire after re-enable");
}

TEST_CASE("Multiple label changes", "[WebButton]") {
  WebButton btn("first");
  btn.SetLabel("second");
  CHECK_MSG(btn.GetLabel() == "second", "label should be second");
  btn.SetLabel("third");
  CHECK_MSG(btn.GetLabel() == "third", "label should be third");
  btn.SetLabel("");
  CHECK_MSG(btn.GetLabel().empty(), "label should be empty");
}

TEST_CASE("SetSize overwrites previous size", "[WebButton]") {
  WebButton btn("test");
  btn.SetSize(100, 50);
  CHECK_MSG(btn.GetWidth() == 100, "width should be 100");
  CHECK_MSG(btn.GetHeight() == 50, "height should be 50");
  btn.SetSize(200, 80);
  CHECK_MSG(btn.GetWidth() == 200, "width should be 200");
  CHECK_MSG(btn.GetHeight() == 80, "height should be 80");
}

TEST_CASE("WebButton is move-only", "[WebButton]") {
  STATIC_REQUIRE(!std::is_copy_constructible_v<WebButton>);
  STATIC_REQUIRE(!std::is_copy_assignable_v<WebButton>);
  STATIC_REQUIRE(std::is_move_constructible_v<WebButton>);
  STATIC_REQUIRE(std::is_move_assignable_v<WebButton>);
}

TEST_CASE("WebButton implements IDomElement", "[WebButton]") {
  STATIC_REQUIRE(std::is_base_of_v<IDomElement, WebButton>);
}

TEST_CASE("HandleClick delegates to Click", "[WebButton]") {
  WebButton btn("test");
  int count = 0;
  btn.SetCallback([&count]() { ++count; });
  btn.HandleClick();
  CHECK_MSG(count == 1, "HandleClick should invoke callback via Click");
}

TEST_CASE("HandleClick respects disabled state", "[WebButton]") {
  WebButton btn("test");
  int count = 0;
  btn.SetCallback([&count]() { ++count; });
  btn.Disable();
  btn.HandleClick();
  CHECK_MSG(count == 0, "HandleClick should not invoke when disabled");
}

TEST_CASE("Multiple rapid clicks all register", "[WebButton]") {
  WebButton btn("test");
  int count = 0;
  btn.SetCallback([&count]() { ++count; });
  for (int i = 0; i < 100; ++i) {
    btn.Click();
  }
  CHECK_MSG(count == 100, "all 100 clicks should register");
}

TEST_CASE("Self move assignment is safe", "[WebButton]") {
  WebButton btn("self");
  btn.SetSize(50, 25);
  btn = std::move(btn);
  CHECK_MSG(btn.GetLabel() == "self", "label should survive self-move");
  CHECK_MSG(btn.GetWidth() == 50, "width should survive self-move");
}

TEST_CASE("unmount is safe to call multiple times", "[WebButton]") {
  WebButton btn("test");
  REQUIRE_NOTHROW(btn.unmount());
  REQUIRE_NOTHROW(btn.unmount());
  REQUIRE_NOTHROW(btn.unmount());
}