// configuration inspired from ArduinoJson - arduinojson.org (Benoit Blanchon 2014-2020)

#pragma once

#include "version.hpp"

#define SANSENSNODE_DO_CONCAT(A, B) A##B
#define SANSENSNODE_CONCAT2(A, B) SANSENSNODE_DO_CONCAT(A, B)
#define SANSENSNODE_CONCAT4(A, B, C, D) \
  SANSENSNODE_CONCAT2(SANSENSNODE_CONCAT2(A, B), SANSENSNODE_CONCAT2(C, D))
#define SANSENSNODE_CONCAT8(A, B, C, D, E, F, G, H)    \
  SANSENSNODE_CONCAT2(SANSENSNODE_CONCAT4(A, B, C, D), \
                      SANSENSNODE_CONCAT4(E, F, G, H))
#define SANSENSNODE_CONCAT12(A, B, C, D, E, F, G, H, I, J, K, L) \
  SANSENSNODE_CONCAT8(A, B, C, D, E, F, G,                       \
                      SANSENSNODE_CONCAT4(H, I, J, SANSENSNODE_CONCAT2(K, L)))

#define SANSENSNODE_NAMESPACE                                            \
  SANSENSNODE_CONCAT4(                                                   \
      SANSENSNODE, SANSENSNODE_VERSION_MAJOR, SANSENSNODE_VERSION_MINOR, \
      SANSENSNODE_VERSION_REVISION)
