// Copyright 2013 Jike Inc. All Rights Reserved.
// Author: liyangguang@jike.com (Yangguang Li)
// File: huffman_compression.h
// Description:
// Created Date: 2013-01-15 15:22:20
#ifndef BASE_HUFFMAN_COMPRESSION_H_
#define BASE_HUFFMAN_COMPRESSION_H_

#include <string>
#include <map>
#include <iterator>

#include "base/gamma_compression.h"
#include "base/varint.h"

namespace base {
const uint32 kNumBitsOfByte = 8;
const uint32 k3BitsMask = 0x7;

struct HuffmanValue {
  uint32 huffman_prefix_len : 16;
  uint32 huffman_value : 16;
};

class HuffmanCompressionConfig {
 public:
  explicit HuffmanCompressionConfig(const std::string& config_file_path);
  bool GetHuffmanCode(const uint64 org_value,
                      const uint8** huffman_code,
                      uint32* huffman_code_len) const;
  bool DecodeHuffmanCode(
      const uint32 huffman_code_value,
      const uint32 huffman_code_len,
      uint64* original_value) const;
  bool GetSpecialHuffmanCode(
      const uint8** special_huffman_code,
      uint32* special_huffman_code_len) const;
  uint32 GetMaxHuffmanCodeLength() const;
  ~HuffmanCompressionConfig();
 private:
  uint32 max_huffman_code_length_;
  std::map<uint64, std::string> huffman_code_map_;
  std::string special_huffman_code_;
  // The index(key) is the prefix path value,
  // and the value is <huffman_code_len, original_value>
  HuffmanValue* huffman_prefix_path_map_;
  uint32 huffman_prefix_path_map_size_;
};

inline bool HuffmanCompressionConfig::GetHuffmanCode(
    const uint64 org_value,
    const uint8** huffman_code,
    uint32* huffman_code_len) const {
  std::map<uint64, std::string>::const_iterator ite =
    huffman_code_map_.find(org_value);
  if (ite == huffman_code_map_.end()) {
    return false;
  }
  *huffman_code = reinterpret_cast<const uint8*>(ite->second.c_str());
  *huffman_code_len = ite->second.size();
  return true;
}

inline bool HuffmanCompressionConfig::DecodeHuffmanCode(
    const uint32 huffman_code_value,
    const uint32 huffman_code_len,
    uint64* original_value) const {
  const HuffmanValue &huffman_org_value =
    huffman_prefix_path_map_[huffman_code_value];
  if (huffman_org_value.huffman_prefix_len == huffman_code_len) {
    *original_value = huffman_org_value.huffman_value;
    return true;
  }
  return false;
}

inline uint32 HuffmanCompressionConfig::GetMaxHuffmanCodeLength() const {
  return max_huffman_code_length_;
}

inline bool HuffmanCompressionConfig::GetSpecialHuffmanCode(
    const uint8** special_huffman_code,
    uint32* special_huffman_code_len) const {
  *special_huffman_code =
    reinterpret_cast<const uint8*>(special_huffman_code_.c_str());
  *special_huffman_code_len = special_huffman_code_.size();
  return true;
}

inline uint8* ZipInt64WithHuffman(
    const HuffmanCompressionConfig* config,
    const uint64* value_list,
    const uint32 value_list_len,
    uint8* target) {
  uint8 buffer_for_varint[kMaxVarintBytes];
  uint32 index = 0;
  DCHECK(config);
  for (int i = 0;i < value_list_len; ++i) {
    const uint8* huffman_code = NULL;
    uint32 huffman_code_len = 0;
    if (config->GetHuffmanCode(
          value_list[i], &huffman_code, &huffman_code_len)) {
      DCHECK(huffman_code_len);
      for (int j = 0; j < huffman_code_len; ++j) {
        if (huffman_code[j] == '0') {
          ClearBit(index++, target);
        } else {
          SetBit(index++, target);
        }
      }
    } else {
      config->GetSpecialHuffmanCode(&huffman_code, &huffman_code_len);
      DCHECK(huffman_code_len);
      // Write the special huffman code as a mark,
      // and the true original value use variable integer to zip.
      for (int j = 0; j < huffman_code_len; ++j) {
        if (huffman_code[j] == '0') {
          ClearBit(index++, target);
        } else {
          SetBit(index++, target);
        }
      }
      // Byte Alignment for quik unzip.
      index = (index + k3BitsMask) & ~(k3BitsMask);
      // Write the variable integer.
      uint8* next_pos = WriteVarint64(value_list[i], buffer_for_varint);
      uint32 varint_size = next_pos - buffer_for_varint;
      for (int j = 0; j < varint_size; ++j) {
        for (int k = 0; k < kNumBitsOfByte; ++k) {
          if ((buffer_for_varint[j] >> k) & 0x1) {
            SetBit(index++, target);
          } else {
            ClearBit(index++, target);
          }
        }
      }
    }
  }
  return target + ((index - 1) >> 3) + 1;
}

inline const uint8* UnzipInt64WithHuffman(
    HuffmanCompressionConfig* config,
    const uint8* buffer,
    uint64* value_list,
    const uint32 value_list_len) {
  uint32 index = 0;
  uint32 max_huffman_code_len = config->GetMaxHuffmanCodeLength();
  // uint8 buffer_for_varint[kMaxVarintBytes];
  for (int i = 0; i < value_list_len; ++i) {
    uint32 huffman_code_value = 0;
    uint32 huffman_code_len = 0;
    bool is_decoded = false;
    while (huffman_code_len < max_huffman_code_len) {
      huffman_code_value += ReadBit(index++, buffer) << huffman_code_len;
      ++huffman_code_len;
      if (config->DecodeHuffmanCode(
            huffman_code_value, huffman_code_len, &value_list[i])) {
        is_decoded = true;
        break;
      }
    }
    if (!is_decoded) {
      // TODO(Yangguang Li) : Check the huffman_code_value is
      // equal to the speical huffman code.

      // Byte Alignment for quik unzip.
      index = (index + k3BitsMask) & ~(k3BitsMask);
      const uint8* next_pos = ReadVarint64(&buffer[index >> 3], &value_list[i]);
      index = (next_pos - buffer) << 3;
    }
  }
  return buffer + ((index - 1) >> 3) + 1;
}

}  // namespace base
#endif  // BASE_HUFFMAN_COMPRESSION_H_
