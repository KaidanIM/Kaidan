# SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
#
# SPDX-License-Identifier: CC0-1.0

include(ECMAddTests)

find_package(Qt5Test REQUIRED)

ecm_add_test(
	PresenceCacheTest.cpp
	../src/PresenceCache.cpp
	TEST_NAME PresenceCacheTest
	LINK_LIBRARIES Qt5::Test Qt5::Gui QXmpp::QXmpp
)

ecm_add_test(
	UserPresenceModelTest.cpp
	../src/PresenceCache.cpp
	TEST_NAME UserPresenceModelTest
	LINK_LIBRARIES Qt5::Test Qt5::Gui QXmpp::QXmpp
)
