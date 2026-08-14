# SPDX-License-Identifier: LicenseRef-CSSL-1.0

add_executable(test_nbiot_tx test_nbiot_tx.c)
target_link_libraries(test_nbiot_tx PRIVATE PHY PHY_COMMON lte_rrc LOG minimal_lib -lm)
add_dependencies(tests test_nbiot_tx)
add_test(NAME test_nbiot_tx COMMAND ./test_nbiot_tx)
