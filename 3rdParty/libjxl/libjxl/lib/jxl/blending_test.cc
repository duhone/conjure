// Copyright (c) the JPEG XL Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <cstdint>
#include <vector>

#include "lib/extras/codec.h"
#include "lib/jxl/image_test_utils.h"
#include "lib/jxl/test_utils.h"
#include "lib/jxl/testing.h"

namespace jxl {
namespace {

using ::testing::SizeIs;

TEST(BlendingTest, Crops) {
  const std::vector<uint8_t> compressed =
      jxl::test::ReadTestData("jxl/blending/cropped_traffic_light.jxl");
  CodecInOut decoded;
  ASSERT_TRUE(test::DecodeFile({}, Bytes(compressed), &decoded));
  ASSERT_THAT(decoded.frames, SizeIs(4));

  int i = 0;
  for (const ImageBundle& ib : decoded.frames) {
    std::ostringstream filename;
    filename << "jxl/blending/cropped_traffic_light_frame-" << i << ".png";
    const std::vector<uint8_t> compressed_frame =
        jxl::test::ReadTestData(filename.str());
    CodecInOut frame;
    ASSERT_TRUE(SetFromBytes(Bytes(compressed_frame), &frame));
    JXL_EXPECT_OK(SamePixels(ib.color(), *frame.Main().color(), _));
    ++i;
  }
}

}  // namespace
}  // namespace jxl
