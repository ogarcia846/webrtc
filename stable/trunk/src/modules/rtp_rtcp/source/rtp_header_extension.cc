/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <cassert>

#include "common_types.h"
#include "rtp_header_extension.h"

namespace webrtc {

RtpHeaderExtensionMap::RtpHeaderExtensionMap() {
}

RtpHeaderExtensionMap::~RtpHeaderExtensionMap() {
  Erase();
}

void RtpHeaderExtensionMap::Erase() {
  while (!extensionMap_.empty()) {
    std::map<uint8_t, HeaderExtension*>::iterator it =
        extensionMap_.begin();
    delete it->second;
    extensionMap_.erase(it);
  }
}

int32_t RtpHeaderExtensionMap::Register(const RTPExtensionType type,
                                        const uint8_t id) {
  if (id < 1 || id > 14) {
    return -1;
  }
  std::map<uint8_t, HeaderExtension*>::iterator it =
      extensionMap_.find(id);
  if (it != extensionMap_.end()) {
    return -1;
  }
  extensionMap_[id] = new HeaderExtension(type);
  return 0;
}

int32_t RtpHeaderExtensionMap::Deregister(const RTPExtensionType type) {
  uint8_t id;
  if (GetId(type, &id) != 0) {
    return -1;
  }
  std::map<uint8_t, HeaderExtension*>::iterator it =
      extensionMap_.find(id);
  if (it == extensionMap_.end()) {
    return -1;
  }
  delete it->second;
  extensionMap_.erase(it);
  return 0;
}

int32_t RtpHeaderExtensionMap::GetType(const uint8_t id,
                                       RTPExtensionType* type) const {
  assert(type);
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.find(id);
  if (it == extensionMap_.end()) {
    return -1;
  }
  HeaderExtension* extension = it->second;
  *type = extension->type;
  return 0;
}

int32_t RtpHeaderExtensionMap::GetId(const RTPExtensionType type,
                                     uint8_t* id) const {
  assert(id);
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.begin();

  while (it != extensionMap_.end()) {
    HeaderExtension* extension = it->second;
    if (extension->type == type) {
      *id = it->first;
      return 0;
    }
    it++;
  }
  return -1;
}

uint16_t RtpHeaderExtensionMap::GetTotalLengthInBytes() const {
  // Get length for each extension block.
  uint16_t length = 0;
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.begin();
  while (it != extensionMap_.end()) {
    HeaderExtension* extension = it->second;
    length += extension->length;
    it++;
  }
  // Add RTP extension header length.
  if (length > 0) {
    length += RTP_ONE_BYTE_HEADER_LENGTH_IN_BYTES;
  }
  return length;
}

int32_t RtpHeaderExtensionMap::GetLengthUntilBlockStartInBytes(
    const RTPExtensionType type) const {
  uint8_t id;
  if (GetId(type, &id) != 0) {
    // Not registered.
    return -1;
  }
  // Get length until start of extension block type.
  uint16_t length = RTP_ONE_BYTE_HEADER_LENGTH_IN_BYTES;

  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.begin();
  while (it != extensionMap_.end()) {
    HeaderExtension* extension = it->second;
    if (extension->type == type) {
      break;
    } else {
      length += extension->length;
    }
    it++;
  }
  return length;
}

int32_t RtpHeaderExtensionMap::Size() const {
  return extensionMap_.size();
}

RTPExtensionType RtpHeaderExtensionMap::First() const {
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.begin();
  if (it == extensionMap_.end()) {
     return kRtpExtensionNone;
  }
  HeaderExtension* extension = it->second;
  return extension->type;
}

RTPExtensionType RtpHeaderExtensionMap::Next(RTPExtensionType type) const {
  uint8_t id;
  if (GetId(type, &id) != 0) {
    return kRtpExtensionNone;
  }
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.find(id);
  if (it == extensionMap_.end()) {
    return kRtpExtensionNone;
  }
  it++;
  if (it == extensionMap_.end()) {
    return kRtpExtensionNone;
  }
  HeaderExtension* extension = it->second;
  return extension->type;
}

void RtpHeaderExtensionMap::GetCopy(RtpHeaderExtensionMap* map) const {
  assert(map);
  std::map<uint8_t, HeaderExtension*>::const_iterator it =
      extensionMap_.begin();
  while (it != extensionMap_.end()) {
    HeaderExtension* extension = it->second;
    map->Register(extension->type, it->first);
    it++;
  }
}
} // namespace webrtc
