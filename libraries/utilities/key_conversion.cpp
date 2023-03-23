/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <hive/utilities/key_conversion.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/variant.hpp>

namespace hive { namespace utilities {

fc::optional<fc::ecc::private_key> wif_to_key( const std::string& wif_key )
{
  std::vector<char> wif_bytes;
  try
  {
    wif_bytes = fc::from_base58(wif_key);
  }
  catch (const fc::parse_error_exception&)
  {
    return fc::optional<fc::ecc::private_key>();
  }
  if (wif_bytes.size() < 5)
    return fc::optional<fc::ecc::private_key>();
  std::vector<char> key_bytes(wif_bytes.begin() + 1, wif_bytes.end() - 4);
  fc::ecc::private_key key = fc::variant(key_bytes).as<fc::ecc::private_key>();
  fc::sha256 check = fc::sha256::hash(wif_bytes.data(), wif_bytes.size() - 4);
  fc::sha256 check2 = fc::sha256::hash(check);

  if( memcmp( (char*)&check, wif_bytes.data() + wif_bytes.size() - 4, 4 ) == 0 ||
      memcmp( (char*)&check2, wif_bytes.data() + wif_bytes.size() - 4, 4 ) == 0 )
    return key;

  return fc::optional<fc::ecc::private_key>();
}

} } // end namespace hive::utilities
