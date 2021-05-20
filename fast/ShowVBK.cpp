#include "pch.h"
#include "ShowVBK.h"

#include "cryptopp820/modes.h"
#include "cryptopp820/base64.h"
#include "cryptopp820/aes.h"
#include "cryptopp820/hex.h"
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "cryptopp820/md5.h"

#include <iostream>
#include <vector>

using byte = unsigned char;

#pragma comment(lib, "cryptlib.lib")
using namespace CryptoPP;

std::string base64de(const std::string& input) {
  std::string result;
  auto encoder = new Base64Decoder(new StringSink(result));
  StringSource(input, true, encoder);
  return std::string(result.begin(), result.end());
}

std::string getLevelStr(int level) {
  std::string levelStr;
  for (int i = 0; i < level; i++) {
    levelStr += "    ";  //这里可以\t换成你所需要缩进的空格数
  }
  return levelStr;
}

std::string formatJson(const std::string& json) {
  std::string result;
  int level = 0;
  for (std::string::size_type index = 0; index < json.size(); index++) {
    char c = json[index];

    if (level > 0 && '\n' == json[json.size() - 1]) {
      result += getLevelStr(level);
    }

    switch (c) {
      case '{':
      case '[':
        result = result + c + "\n";
        level++;
        result += getLevelStr(level);
        break;
      case ',':
        result = result + c + "\n";
        result += getLevelStr(level);
        break;
      case '}':
      case ']':
        result += "\n";
        level--;
        result += getLevelStr(level);
        result += c;
        break;
      default:
        result += c;
        break;
    }
  }
  return result;
}

std::string CBC_AESDecryptStr(const std::string& input) {
  static std::string sKey("aonmwbgaxoefbvcs");
  static std::string sIV(base64de("dmZkZ25ncmhhaHppYmFjYg=="));
  std::string buffer(base64de(input));
  //填key
  SecByteBlock key(0x00, AES::DEFAULT_KEYLENGTH);
  sKey.size() <= AES::DEFAULT_KEYLENGTH
      ? memcpy(key, sKey.c_str(), sKey.size())
      : memcpy(key, sKey.c_str(), AES::DEFAULT_KEYLENGTH);

  //填iv
  byte iv[AES::BLOCKSIZE];
  memset(iv, 0, AES::BLOCKSIZE);
  sIV.size() <= AES::BLOCKSIZE ? memcpy(iv, sIV.data(), sIV.size())
                               : memcpy(iv, sIV.data(), AES::BLOCKSIZE);

  CBC_Mode<AES>::Decryption cbcDecryption((byte*)key, AES::DEFAULT_KEYLENGTH,
                                          iv);
  std::string outstr;
  StreamTransformationFilter decryptor(cbcDecryption, new StringSink(outstr));
  decryptor.Put((byte*)buffer.c_str(), buffer.size());
  decryptor.MessageEnd();

  return outstr;
}

bool DecodeVbk(const std::string& input, std::string *output) {
  std::string result = CBC_AESDecryptStr(input);
  output->assign(result);
  return !result.empty();
}

int ShowVBK(LPCWSTR FilePath,
            CWindow& RichEdit,
            CAtlFileMapping<char>& FileMapping) {
  std::string result;
  if (!DecodeVbk((LPCSTR)FileMapping, &result)) {
    return LISTPLUGIN_ERROR;
  }
  SetWindowTextA(RichEdit, formatJson(result).c_str());
  return LISTPLUGIN_OK;
}
