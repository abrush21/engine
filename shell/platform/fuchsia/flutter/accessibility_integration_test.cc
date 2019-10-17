// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "topaz/runtime/flutter_runner/accessibility_bridge.h"

#include <gtest/gtest.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/fidl/cpp/binding_set.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/gtest/real_loop_fixture.h>
#include <lib/sys/cpp/testing/component_context_provider.h>
#include <lib/sys/cpp/testing/service_directory_provider.h>
#include <lib/syslog/cpp/logger.h>
#include <zircon/status.h>
#include <zircon/types.h>

#include <memory>

#include "src/ui/a11y/lib/semantics/semantics_manager.h"

namespace flutter_runner_test {
class AccessibilityFlutterIntegrationTest : public gtest::RealLoopFixture {
 public:
  AccessibilityFlutterIntegrationTest() : context_provider_(dispatcher()),
					  semantics_manager_(context_provider_.context()) {
   RunLoopUntilIdle();
   
   context_provider_.service_directory_provider()->AddService(semantics_manager_.GetHandler(dispatcher()), fuchsia::accessibility::semantics::SemanticsManager::Name_);
   RunLoopUntilIdle();
  }

  

 protected:

  void SetUp() override {
    RealLoopFixture::SetUp();
    zx::eventpair a, b;
    zx::eventpair::create(0u, &a, &b);
    view_ref_ = fuchsia::ui::views::ViewRef({
        .reference = std::move(a),
    });
  }

  fuchsia::ui::views::ViewRef Clone(const fuchsia::ui::views::ViewRef& view_ref) {
	  fuchsia::ui::views::ViewRef clone;
	  FX_CHECK(fidl::Clone(view_ref, &clone) == ZX_OK);
	  return clone;
  }

  sys::testing::ComponentContextProvider context_provider_;
  a11y::SemanticsManager semantics_manager_;
  std::unique_ptr<flutter_runner::AccessibilityBridge> accessibility_bridge_;
  fuchsia::ui::views::ViewRef view_ref_;
};

TEST_F(AccessibilityFlutterIntegrationTest, TestSetup) {
  // Constructor attempts to connect to semantic manager service using 
  // service name SemanticsManager::Name_, and then calls RegisterViewForSemantics().
  accessibility_bridge_ =
      std::make_unique<flutter_runner::AccessibilityBridge>(
          context_provider_.service_directory_provider()->service_directory(), Clone(view_ref_));
  RunLoopUntilIdle();

  flutter::SemanticsNode node0;
  node0.id = 0;

  accessibility_bridge_->AddSemanticsNodeUpdate({{0, node0}});
  RunLoopUntilIdle();

  std::string semantic_tree_log = semantics_manager_.LogSemanticTreeForView(view_ref_);

  EXPECT_FALSE(semantic_tree_log.empty());
}

}  // namespace flutter_runner_test
